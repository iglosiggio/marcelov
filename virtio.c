#include <stddef.h>
#include <stdint.h>

#include "utils.h"

struct virtio_device {
	// "virt"
	const uint32_t magic;
	// 0x1 for legacy devices, 0x2 otherwise
	const uint32_t version;
	// https://docs.oasis-open.org/virtio/virtio/v1.2/csd01/virtio-v1.2-csd01.html#x1-2160005
	const uint32_t device_id;
	const uint32_t vendor_id;
	// Supported features for the selected feature set (see
	// device_features_selector)
	const uint32_t device_features;
	// Selects which feature set to make visible at device_features
	uint32_t device_features_selector;
	char unused0[8];
	// Selects which features this driver uses for the selected feature set
	// (see driver_features_selector)
	uint32_t driver_features;
	// Selects the feature set to configure by writing at driver_features
	uint32_t driver_features_selector;
	char unused1[8];
	// Selects the virtqueue to configure
	uint32_t queue_selector;
	// Maximum amount of elements the queue is ready to process for the
	// selected virtqueue (0x0 means not available)
	const uint32_t queue_max_size;
	// Notifies the device of the queue size for the selected virqueue
	uint32_t queue_size;
	char unused2[8];
	// Marks the selected virtqueue as ready
	uint32_t queue_ready;
	char unused3[8];
	// Notifies the device that there are new buffers to process
	uint32_t queue_notify;
	char unused4[12];
	// Bitmask of events that caused an interrupt to be asserted
	const uint32_t interrupt_status;
	// Acknowledges the interrupt by specifying which events were handled
	uint32_t interrupt_acknowledge;
	char unused5[8];
	// Status of the device
	uint32_t status;
	char unused6[12];
	// Configures the physical address of the descriptor area for the
	// selected virtqueue
	uint32_t queue_descriptor_address_low;
	uint32_t queue_descriptor_address_high;
	char unused7[8];
	// Configures the physical address of the driver area for the selected
	// virtqueue
	uint32_t queue_driver_address_low;
	uint32_t queue_driver_address_high;
	char unused8[8];
	// Configures the physical address of the device area for the selected
	// virtqueue
	uint32_t queue_device_address_low;
	uint32_t queue_device_address_high;
	char unused9[4];
	// Selects the shared memory region to make visible
	uint32_t shmem_selector;
	// Size of the selected shared memory region
	const uint32_t shmem_size_low;
	const uint32_t shmem_size_high;
	// Physical address of the selected shared memory region
	const uint32_t shmem_address_low;
	const uint32_t shmem_address_high;
	// Resets the selected virtqueue when 0x1 is written to it
	uint32_t queue_reset;
	char unused10[0x38];
	// Modified between configuration space changes, allows to perform
	// atomic operations by checking that the generation was stable between
	// the start and end of the configuration space access operations
	const uint32_t config_generation;
	char config[];
};

enum virtio_status_bits {
	/* Indicates that the guest OS has found the device and recognized it
	 * as a valid virtio device. */
	VIRTIO_STATUS_ACK                = 0x01,
	/* Indicates that the guest OS knows how to drive the device. Note:
	 * There could be a significant (or infinite) delay before setting this
	 * bit. For example, under Linux, drivers can be loadable modules. */
	VIRTIO_STATUS_DRIVER             = 0x02,
	/* Indicates that something went wrong in the guest, and it has given
	 * up on the device. This could be an internal error, or the driver
	 * didn't like the device for some reason, or even a fatal error during
	 * device operation. */
	VIRTIO_STATUS_FAILED             = 0x80,
	/* Indicates that the driver has acknowledged all the features it
	 * understands, and feature negotiation is complete. */
	VIRTIO_STATUS_DRIVER_OK          = 0x08,
	/* Indicates that the driver is set up and ready to drive the device.
	 */
	VIRTIO_STATUS_FEATURES_OK        = 0x04,
	/* Indicates that the device has experienced an error from which it
	 * can't recover. */
	VIRTIO_STATUS_DEVICE_NEEDS_RESET = 0x40
};

static inline
bool is_set(uint64_t bitmap, uint64_t bit) {
	return bitmap & bit == bit;
}

static inline
bool is_unset(uint64_t bitmap, uint64_t bit) {
	return ~bitmap & bit == 0;
}

enum virtio_input_config_select {
	VIRTIO_INPUT_CFG_UNSET      = 0x00,
	VIRTIO_INPUT_CFG_ID_NAME    = 0x01,
	VIRTIO_INPUT_CFG_ID_SERIAL  = 0x02,
	VIRTIO_INPUT_CFG_ID_DEVIDS  = 0x03,
	VIRTIO_INPUT_CFG_PROP_BITS  = 0x10,
	VIRTIO_INPUT_CFG_EV_BITS    = 0x11,
	VIRTIO_INPUT_CFG_ABS_INFO   = 0x12
};

struct virtio_input_absinfo {
	uint32_t min;
	uint32_t max;
	uint32_t fuzz;
	uint32_t flat;
	uint32_t res;
};

struct virtio_input_devids {
	uint16_t bustype;
	uint16_t vendor;
	uint16_t product;
	uint16_t verstion;
};

struct virtio_input_config {
	uint8_t select;
	uint8_t subsel;
	uint8_t size;
	uint8_t reserved[5];
	union {
		char string[128];
		uint8_t bitmap[128];
		struct virtio_input_absinfo abs;
		struct virtio_input_devids ids;
	};
};

struct virtio_input_event {
	uint16_t type;
	uint16_t code;
	uint32_t value;
};

volatile struct virtio_device* const virtio_keyboard = (void*) 0x10008000;
volatile struct virtio_device* const virtio_mouse = (void*) 0x10007000;

void virtio_device_dump(volatile struct virtio_device* device) {
	print("Address: ");
	print_hex((uintptr_t) device);
	print("\n");

	uint32_t magic = device->magic;
	print("Magic: ");
	print((char[]) {
		(magic >>  0) & 0xFF,
		(magic >>  8) & 0xFF,
		(magic >> 16) & 0xFF,
		(magic >> 24) & 0xFF,
		0
	});
	print("\n");


	print("Version: ");
	print_hex(device->version);
	print("\n");

	print("Device ID: ");
	print_hex(device->device_id);
	print("\n");

	print("Vendor ID: ");
	print_hex(device->vendor_id);
	print("\n");

	if (device->device_id == 0x12) {
		struct virtio_input_config* config = (void*) device->config;

		config->subsel = 0;
		config->select = VIRTIO_INPUT_CFG_ID_NAME;
		if (config->size != 0) {
			print("Name: ");
			print(config->string);
			print("\n");
		}

		config->subsel = 0;
		config->select = VIRTIO_INPUT_CFG_ID_SERIAL;
		if (config->size != 0) {
			print("Serial: ");
			for (int i = 0; i < config->size; i++) {
				if (i != 0) {
					print(" ");
				}
				print_hex(config->string[i]);
			}
			print("\n");
		}

		config->subsel = 0;
		config->select = VIRTIO_INPUT_CFG_ID_DEVIDS;
		if (config->size != 0) {
			print("Device IDs:");
			print("\n - Bus type: ");
			print_hex(config->ids.bustype);
			print("\n - Vendor: ");
			print_hex(config->ids.vendor);
			print("\n - Product: ");
			print_hex(config->ids.product);
			print("\n - Version: ");
			print_hex(config->ids.bustype);
			print("\n");
		}
	}
}

int virtio_device_init_input_device(volatile struct virtio_device* device) {
	/* The driver MUST start the device initialization by reading and
	 * checking values from MagicValue and Version. If both values are
	 * valid, it MUST read DeviceID and if its value is zero (0x0) MUST
	 * abort initialization and MUST NOT access any other register. */
	if (device->magic != 0x74726976) goto failed;
	if (device->version != 2) goto failed;
	if (device->device_id == 0) goto failed;

	/* 1. Reset the device */
	device->status = 0;
	/* 2. Set the ACKNOWLEDGE status bit: the guest OS has noticed the
	 * device. */
	device->status |= VIRTIO_STATUS_ACK;
	/* 3. Set the DRIVER status bit: the guest OS knows how to drive the
	 * device. */
	device->status |= VIRTIO_STATUS_DRIVER;
	/* 4. Read device feature bits, and write the subset of feature bits
	 * understood by the OS and driver to the device. During this step the
	 * driver MAY read (but MUST NOT write) the device-specific
	 * configuration fields to check that it can support the device before
	 * accepting it. */
	// NOTE: Input devices have no feature bits, we do the read/write thing
	// just in case.
	device->device_features_selector = 0;
	(void) device->device_features;
	device->driver_features_selector = 0;
	device->driver_features = 0;
	/* Set the FEATURES_OK status bit. The driver MUST NOT accept new
	 * feature bits after this step. */
	device->status |= VIRTIO_STATUS_FEATURES_OK;
	/* 5. Re-read device status to ensure the FEATURES_OK bit is still set:
	 * otherwise, the device does not support our subset of features and
	 * the device is unusable. */
	if (is_unset(device->status, VIRTIO_STATUS_FEATURES_OK)) return -1;
	/* 6. Perform device-specific setup, including discovery of virtqueues
	 * for the device, optional per-bus setup, reading and possibly writing
	 * the device’s virtio configuration space, and population of
	 * virtqueues. */
	// TODO
	/* 7. Set the DRIVER_OK status bit. At this point the device is “live”.
	 */
	device->status |= VIRTIO_STATUS_DRIVER_OK;
	return 0;
failed:
	/* If any of these steps go irrecoverably wrong, the driver SHOULD set
	 * the FAILED status bit to indicate that it has given up on the device
	 * (it can reset the device later to restart if desired). The driver
	 * MUST NOT continue initialization in that case. */
	device->status |= VIRTIO_STATUS_FAILED;
	return -1;
}

int virtio_device_configure_virtqueue(volatile struct virtio_device* device, uint32_t selector, void* queue_buf, uint64_t queue_size) {
	/* 1. Select the queue writing its index (first queue is 0) to
	 * QueueSel. */
	device->queue_selector = selector;
	/* 2. Check if the queue is not already in use: read QueueReady, and
	 * expect a returned value of zero (0x0). */
	if (device->queue_ready != 0) return -1;
	/* 3. Read maximum queue size (number of elements) from QueueNumMax. If
	 * the returned value is zero (0x0) the queue is not available. */
	if (device->queue_max_size == 0) return -1;
	/* 4. Allocate and zero the queue memory, making sure the memory is
	 * physically contiguous. */
	if (device->queue_max_size < queue_size) {
		queue_size = device->queue_max_size;
	}
	uint64_t queue_buf_size = 16 * queue_size     /* Descriptor table */
	                        + 6 + 2 * queue_size  /* Available ring */
	                        + 6 + 8 * queue_size; /* Used ring */
	memset(queue_buf, queue_buf_size, 0);
	/* 5. Notify the device about the queue size by writing the size to
	 * QueueNum. */
	device->queue_size = queue_size;
	/* 6. Write physical addresses of the queue’s Descriptor Area, Driver
	 * Area and Device Area to (respectively) the
	 * QueueDescLow/QueueDescHigh, QueueDriverLow/QueueDriverHigh and
	 * QueueDeviceLow/QueueDeviceHigh register pairs. */


	//// Configures the physical address of the descriptor area for the
	//// selected virtqueue
	//uint32_t queue_descriptor_address_low;
	//uint32_t queue_descriptor_address_high;
	//char unused7[8];
	//// Configures the physical address of the driver area for the selected
	//// virtqueue
	//uint32_t queue_driver_address_low;
	//uint32_t queue_driver_address_high;
	//char unused8[8];
	//// Configures the physical address of the device area for the selected
	//// virtqueue
	//uint32_t queue_device_address_low;
	//uint32_t queue_device_address_high;

	/* 7. Write 0x1 to QueueReady. */
	device->queue_ready = 1;
	return 0;
}
