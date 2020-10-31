#include "usb.h"

#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/dmamux.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/stm32/crs.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/desig.h>

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/usb/midi.h>
#include <libopencm3/usb/audio.h>
#include <libopencm3/usb/hid.h>

#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>

usbd_device *usbd_fs_device;

#define INTERFACE_COUNT 4

static uint8_t usbd_control_buffer[256];

enum {
    INTERFACE_MIDI_STREAMING = 0,
	INTERFACE_CDC_COMM = 1,
    INTERFACE_CDC_DATA = 2,
    INTERFACE_AUDIO_CONTROL = 3,
};

enum {
    ENDPOINT_CDC_COMM_IN = 0x82,
    ENDPOINT_CDC_DATA_IN = 0x81,
    ENDPOINT_CDC_DATA_OUT = 0x01,
    ENDPOINT_MIDI_DATA_IN = 0x83,
    ENDPOINT_MIDI_DATA_OUT= 0x02,
};


static const struct usb_device_descriptor device_descriptor = {
    // The size of this descriptor in bytes, 18.
    .bLength = USB_DT_DEVICE_SIZE,
    // A value of 1 indicates that this is a device descriptor.
    .bDescriptorType = USB_DT_DEVICE,
    // This device supports USB 2.0
    .bcdUSB = 0x0200,
    .bDeviceClass = 0xEF, // Miscellaneous Device.
    .bDeviceSubClass = 2, // Common Class
    .bDeviceProtocol = 1, // Interface Association
    .bMaxPacketSize0 = 64,
    .idVendor = 0x6666, // VID reserved for prototypes
    .idProduct = 0x0001,
    // Version number for the device. Set to 1.0.0 for now.
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};


/*
 * Midi specific endpoint descriptors.
 */
static const struct usb_midi_endpoint_descriptor midi_bulk_endp[] = {{
	/* Table B-12: MIDI Adapter Class-specific Bulk OUT Endpoint
	 * Descriptor
	 */
	.head = {
		.bLength = sizeof(struct usb_midi_endpoint_descriptor),
		.bDescriptorType = USB_AUDIO_DT_CS_ENDPOINT,
		.bDescriptorSubType = USB_MIDI_SUBTYPE_MS_GENERAL,
		.bNumEmbMIDIJack = 1,
	},
	.jack[0] = {
		.baAssocJackID = 0x01,
	},
}, {
	/* Table B-14: MIDI Adapter Class-specific Bulk IN Endpoint
	 * Descriptor
	 */
	.head = {
		.bLength = sizeof(struct usb_midi_endpoint_descriptor),
		.bDescriptorType = USB_AUDIO_DT_CS_ENDPOINT,
		.bDescriptorSubType = USB_MIDI_SUBTYPE_MS_GENERAL,
		.bNumEmbMIDIJack = 1,
	},
	.jack[0] = {
		.baAssocJackID = 0x03,
	},
} };

/*
 * Standard endpoint descriptors
 */
static const struct usb_endpoint_descriptor bulk_endp[] = {{
	/* Table B-11: MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = ENDPOINT_MIDI_DATA_OUT,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 0x40,
	.bInterval = 0x00,

	.extra = &midi_bulk_endp[0],
	.extralen = sizeof(midi_bulk_endp[0])
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = ENDPOINT_MIDI_DATA_IN,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 0x40,
	.bInterval = 0x00,

	.extra = &midi_bulk_endp[1],
	.extralen = sizeof(midi_bulk_endp[1])
} };


static const struct {
	struct usb_audio_header_descriptor_head header_head;
	struct usb_audio_header_descriptor_body header_body;
} __attribute__((packed)) audio_control_functional_descriptors = {
	.header_head = {
		.bLength = sizeof(struct usb_audio_header_descriptor_head) +
		           1 * sizeof(struct usb_audio_header_descriptor_body),
		.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
		.bDescriptorSubtype = USB_AUDIO_TYPE_HEADER,
		.bcdADC = 0x0100,
		.wTotalLength =
			   sizeof(struct usb_audio_header_descriptor_head) +
			   1 * sizeof(struct usb_audio_header_descriptor_body),
		.binCollection = 1,
	},
	.header_body = {
		.baInterfaceNr = 0x01,
	},
};

/*
 * Table B-3: MIDI Adapter Standard AC Interface Descriptor
 */
static const struct usb_interface_descriptor audio_control_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = INTERFACE_AUDIO_CONTROL,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0,
	.bInterfaceClass = USB_CLASS_AUDIO,
	.bInterfaceSubClass = USB_AUDIO_SUBCLASS_CONTROL,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.extra = &audio_control_functional_descriptors,
	.extralen = sizeof(audio_control_functional_descriptors)
};

/*
 * Class-specific MIDI streaming interface descriptor
 */
static const struct {
	struct usb_midi_header_descriptor header;
	struct usb_midi_in_jack_descriptor in_embedded;
	struct usb_midi_in_jack_descriptor in_external;
	struct usb_midi_out_jack_descriptor out_embedded;
	struct usb_midi_out_jack_descriptor out_external;
} __attribute__((packed)) midi_streaming_functional_descriptors = {
	/* Table B-6: Midi Adapter Class-specific MS Interface Descriptor */
	.header = {
		.bLength = sizeof(struct usb_midi_header_descriptor),
		.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
		.bDescriptorSubtype = USB_MIDI_SUBTYPE_MS_HEADER,
		.bcdMSC = 0x0100,
		.wTotalLength = sizeof(midi_streaming_functional_descriptors),
	},
	/* Table B-7: MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
	.in_embedded = {
		.bLength = sizeof(struct usb_midi_in_jack_descriptor),
		.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
		.bDescriptorSubtype = USB_MIDI_SUBTYPE_MIDI_IN_JACK,
		.bJackType = USB_MIDI_JACK_TYPE_EMBEDDED,
		.bJackID = 0x01,
		.iJack = 0x00,
	},
	/* Table B-8: MIDI Adapter MIDI IN Jack Descriptor (External) */
	.in_external = {
		.bLength = sizeof(struct usb_midi_in_jack_descriptor),
		.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
		.bDescriptorSubtype = USB_MIDI_SUBTYPE_MIDI_IN_JACK,
		.bJackType = USB_MIDI_JACK_TYPE_EXTERNAL,
		.bJackID = 0x02,
		.iJack = 0x00,
	},
	/* Table B-9: MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
	.out_embedded = {
		.head = {
			.bLength = sizeof(struct usb_midi_out_jack_descriptor),
			.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_MIDI_SUBTYPE_MIDI_OUT_JACK,
			.bJackType = USB_MIDI_JACK_TYPE_EMBEDDED,
			.bJackID = 0x03,
			.bNrInputPins = 1,
		},
		.source[0] = {
			.baSourceID = 0x02,
			.baSourcePin = 0x01,
		},
		.tail = {
			.iJack = 0x00,
		}
	},
	/* Table B-10: MIDI Adapter MIDI OUT Jack Descriptor (External) */
	.out_external = {
		.head = {
			.bLength = sizeof(struct usb_midi_out_jack_descriptor),
			.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_MIDI_SUBTYPE_MIDI_OUT_JACK,
			.bJackType = USB_MIDI_JACK_TYPE_EXTERNAL,
			.bJackID = 0x04,
			.bNrInputPins = 1,
		},
		.source[0] = {
			.baSourceID = 0x01,
			.baSourcePin = 0x01,
		},
		.tail = {
			.iJack = 0x00,
		},
	},
};

/*
 * Table B-5: MIDI Adapter Standard MS Interface Descriptor
 */
static const struct usb_interface_descriptor midi_streaming_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = INTERFACE_MIDI_STREAMING,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_AUDIO,
	.bInterfaceSubClass = USB_AUDIO_SUBCLASS_MIDISTREAMING,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = bulk_endp,

	.extra = &midi_streaming_functional_descriptors,
	.extralen = sizeof(midi_streaming_functional_descriptors)
};

static const struct usb_endpoint_descriptor cdc_comm_endpoints[] = {
    {
        // The size of the endpoint descriptor in bytes: 7.
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = ENDPOINT_CDC_COMM_IN,
        .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
        .wMaxPacketSize = 16,
        .bInterval = 255,
    }
};

static const struct usb_endpoint_descriptor cdc_data_endpoints[] = {
    {
        // The size of the endpoint descriptor in bytes: 7.
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = ENDPOINT_CDC_DATA_OUT,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 1,
    },
    {
        // The size of the endpoint descriptor in bytes: 7.
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = ENDPOINT_CDC_DATA_IN,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 1,
    }
};

static const struct {
    struct usb_cdc_header_descriptor header;
    struct usb_cdc_call_management_descriptor call_mgmt;
    struct usb_cdc_acm_descriptor acm;
    struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
    .header = {
        // The size of the CDC header descriptor: 5.
        .bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
       .bcdCDC = 0x0110,
    },
    .call_mgmt = {
        // The length of this descriptor: 5.
        .bFunctionLength = sizeof(struct usb_cdc_call_management_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
        .bmCapabilities = 0,
        .bDataInterface = INTERFACE_CDC_DATA,
    },
    .acm = {
        // The size of this descriptor: 4.
        .bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_ACM,
        .bmCapabilities = 0,
    },
    .cdc_union = {
        // The length of this descriptor: 5.
        .bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_UNION,
        .bControlInterface = 1,
        .bSubordinateInterface0 = 2,
    }
};

static const struct usb_interface_descriptor cdc_comm_interface = {
    // The size of an interface descriptor: 9
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = INTERFACE_CDC_COMM,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_CDC,
    .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
    .bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
    .iInterface = 0,
    .endpoint = cdc_comm_endpoints,
    .extra = &cdcacm_functional_descriptors,
    .extralen = sizeof(cdcacm_functional_descriptors)
};

static const struct usb_interface_descriptor cdc_data_interface = {
    // The size of an interface descriptor: 9
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = INTERFACE_CDC_DATA,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_DATA,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,

    .endpoint = cdc_data_endpoints,
};

static const struct usb_iface_assoc_descriptor cdc_acm_interface_association = {
    .bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
    .bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
    .bFirstInterface = INTERFACE_CDC_COMM,
    .bInterfaceCount = 2,
    .bFunctionClass = USB_CLASS_CDC,
    .bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
    .bFunctionProtocol = USB_CDC_PROTOCOL_AT,
    .iFunction = 0,
};




const struct usb_interface interfaces[] = {
	{
        .num_altsetting = 1,
        .altsetting = &audio_control_iface,
    },
	{
        .num_altsetting = 1,
        .altsetting = &midi_streaming_iface,
    },
    {
        .num_altsetting = 1,
        .iface_assoc = &cdc_acm_interface_association,
        .altsetting = &cdc_comm_interface,
    },
    {
        .num_altsetting = 1,
        .altsetting = &cdc_data_interface,
    },
};

static const struct usb_config_descriptor config_descriptor = {
    // The length of this header in bytes, 9.
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0,
    .bNumInterfaces = INTERFACE_COUNT,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0b10000000,
    .bMaxPower = 50,
    .interface = interfaces,
};

// The string table.
static const char *usb_strings[] = {
    "Vojtech Vosahlo",
    "MIDIControl",
	"000000000001"
};


static enum usbd_request_return_codes usb_cdc_control(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,uint16_t *len, void (**complete)(usbd_device *usbd_dev,						    struct usb_setup_data *req))
{
  (void)complete;
  (void)buf;
  (void)usbd_dev;

  switch(req->bRequest) {
  case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
    /*
     * This Linux cdc_acm driver requires this to be implemented
     * even though it's optional in the CDC spec, and we don't
     * advertise it in the ACM functional descriptor.
     */
    char local_buf[10];
    struct usb_cdc_notification *notif = (void *)local_buf;

    /* We echo signals back to host as notification. */
    notif->bmRequestType = 0xA1;
    notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
    notif->wValue = 0;
    notif->wIndex = 0;
    notif->wLength = 2;
    local_buf[8] = req->wValue & 3;
    local_buf[9] = 0;
    // usbd_ep_write_packet(0x83, buf, 10);
    return USBD_REQ_HANDLED;
  }
  case USB_CDC_REQ_SET_LINE_CODING: 
    if(*len < sizeof(struct usb_cdc_line_coding)) {
      return USBD_REQ_NOTSUPP;
    }
    return USBD_REQ_HANDLED;
  }
  return 0;
}




/* SysEx identity message, preformatted with correct USB framing information */
const uint8_t sysex_identity[] = {
	0x04,	/* USB Framing (3 byte SysEx) */
	0xf0,	/* SysEx start */
	0x7e,	/* non-realtime */
	0x00,	/* Channel 0 */
	0x04,	/* USB Framing (3 byte SysEx) */
	0x7d,	/* Educational/prototype manufacturer ID */
	0x66,	/* Family code (byte 1) */
	0x66,	/* Family code (byte 2) */
	0x04,	/* USB Framing (3 byte SysEx) */
	0x51,	/* Model number (byte 1) */
	0x19,	/* Model number (byte 2) */
	0x00,	/* Version number (byte 1) */
	0x04,	/* USB Framing (3 byte SysEx) */
	0x00,	/* Version number (byte 2) */
	0x01,	/* Version number (byte 3) */
	0x00,	/* Version number (byte 4) */
	0x05,	/* USB Framing (1 byte SysEx) */
	0xf7,	/* SysEx end */
	0x00,	/* Padding */
	0x00,	/* Padding */
};

static void usb_midi_rx(usbd_device *dev, uint8_t ep){
	(void)ep;

	char buf[64];
	int len = usbd_ep_read_packet(dev, ENDPOINT_MIDI_DATA_OUT, buf, 64);

	/*if(len){
		usbd_ep_write_packet(dev, ENDPOINT_MIDI_DATA_IN, sysex_identity, sizeof(sysex_identity));
	}*/
}

static void usb_cdc_rx(usbd_device *usbd_dev, uint8_t ep){
	(void)ep;
	char buf[64];
	int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);

	if (len) {
		usbd_ep_write_packet(usbd_dev, 0x82, buf, len);
		buf[len] = 0;
	}
}

uint32_t usb_cdc_tx(void *buf, int len){
	return usbd_ep_write_packet(usbd_fs_device, ENDPOINT_CDC_DATA_IN, buf, len);
}

uint32_t usb_midi_tx(void *buf, int len){
	return usbd_ep_write_packet(usbd_fs_device, ENDPOINT_MIDI_DATA_IN, buf, len);
}

static int cdcacm_control_request_handler(
    usbd_device *dev,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *, struct usb_setup_data *)) {

    (void)dev;
    (void)buf;
    (void)complete;

    if (req->bRequest == USB_CDC_REQ_SET_CONTROL_LINE_STATE) {
        // The Linux cdc_acm driver requires this to be implemented even though
        // it's optional in the CDC spec, and we don't advertise it in the ACM
        // functional descriptor.
        return USBD_REQ_HANDLED;
    } else if (req->bRequest == USB_CDC_REQ_SET_LINE_CODING) {    
        if(*len < sizeof(struct usb_cdc_line_coding)) {
            return USBD_REQ_NOTSUPP;
        }
        return USBD_REQ_HANDLED;
    }

    return USBD_REQ_NOTSUPP;
}

// This function is called when the target is an interface.
static int interface_control_request_handler(
    usbd_device *dev,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *, struct usb_setup_data *)) {

   /* if (req->wIndex == INTERFACE_RAW_HID) {
        return raw_hid_control_request_handler(dev, req, buf, len, complete);
    }*/

    if (req->wIndex == INTERFACE_CDC_COMM) {
        return cdcacm_control_request_handler(dev, req, buf, len, complete);
    }

    /*if (req->wIndex == INTERFACE_KEYBOARD_HID) {
        return keyboard_hid_control_request_handler(
            dev, req, buf, len, complete);
    }*/

    // This handler didn't handle this command, try the next one.
    return USBD_REQ_NEXT_CALLBACK;
}

static void set_config_handler(usbd_device *dev, uint16_t wValue) {
    (void)dev;
    (void)wValue;

    // CDC endpoints:
    // OUT endpoint for data.
    usbd_ep_setup(dev, ENDPOINT_CDC_DATA_OUT, USB_ENDPOINT_ATTR_BULK, 64, usb_cdc_rx);
    // IN endpoint for data.
    usbd_ep_setup(dev, ENDPOINT_CDC_DATA_IN, USB_ENDPOINT_ATTR_BULK, 64, NULL);
    // Useless IN endpoint for comm.
    // TODO: Can this be smaller?
    usbd_ep_setup(dev, ENDPOINT_CDC_COMM_IN, USB_ENDPOINT_ATTR_INTERRUPT, 8, NULL);

	//MIDI endpoints:
	usbd_ep_setup(dev, ENDPOINT_MIDI_DATA_OUT, USB_ENDPOINT_ATTR_BULK, 64, usb_midi_rx);
	usbd_ep_setup(dev, ENDPOINT_MIDI_DATA_IN, USB_ENDPOINT_ATTR_BULK, 64, NULL);


    // This callback is registered for requests that are sent to an interface.
    // It does this by applying the mask to bmRequestType and making sure it is
    // equal to the supplied value.
    usbd_register_control_callback(dev, USB_REQ_TYPE_INTERFACE, USB_REQ_TYPE_RECIPIENT, interface_control_request_handler);

	 //usbd_register_control_callback(dev, USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE, USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT, usb_cdc_control);
}

void usb_init(){
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_USB_DM | GPIO_USB_DP);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO_USB_DM | GPIO_USB_DP);

    nvic_enable_irq(NVIC_USB_LP_IRQ);
	
    crs_autotrim_usb_enable();
	rcc_set_clock48_source(RCC_CCIPR_CLK48_HSI48);

	rcc_osc_on(RCC_HSI48);
	rcc_wait_for_osc_ready(RCC_HSI48);

	usbd_fs_device = usbd_init(&st_usbfs_v2_usb_driver, &device_descriptor, &config_descriptor, usb_strings, sizeof(usb_strings), usbd_control_buffer, sizeof(usbd_control_buffer));

	usbd_register_set_config_callback(usbd_fs_device, set_config_handler);

   
}

void usb_lp_isr(void){
    usbd_poll(usbd_fs_device);
}
