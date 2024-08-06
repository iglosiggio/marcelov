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
	void* queue_descriptor_address;
	char unused7[8];
	// Configures the physical address of the driver area for the selected
	// virtqueue
	void* queue_driver_address;
	char unused8[8];
	// Configures the physical address of the device area for the selected
	// virtqueue
	void* queue_device_address;
	char unused9[4];
	// Selects the shared memory region to make visible
	uint32_t shmem_selector;
	// Size of the selected shared memory region
	const uint64_t shmem_size;
	// Physical address of the selected shared memory region
	void* const shmem_address;
	// Resets the selected virtqueue when 0x1 is written to it
	uint32_t queue_reset;
	char unused10[0x38];
	// Modified between configuration space changes, allows to perform
	// atomic operations by checking that the generation was stable between
	// the start and end of the configuration space access operations
	const uint32_t config_generation;
	char config[];
};

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

struct virtio_config_input {
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

#define P(field) print("Offset of "#field": "); print_hex(offsetof(struct virtio_device, field)); print("\n")

void test_enumerate() {
	P(magic);
	P(version);
	P(device_id);
	P(vendor_id);
	P(device_features);
	P(device_features_selector);
	P(driver_features);
	P(driver_features_selector);
	P(queue_selector);
	P(queue_max_size);
	P(queue_size);
	P(queue_ready);
	P(queue_notify);
	P(interrupt_status);
	P(interrupt_acknowledge);
	P(status);
	P(queue_descriptor_address);
	P(queue_driver_address);
	P(queue_device_address);
	P(shmem_selector);
	P(shmem_size);
	P(shmem_address);
	P(queue_reset);
	P(config_generation);
	P(config);

	struct virtio_device* devices[] = {
		(void*) 0x10008000, /* keyboard */
		(void*) 0x10007000, /* mouse */
		//(void*) 0x10006000,
		//(void*) 0x10005000,
		//(void*) 0x10004000,
		//(void*) 0x10003000,
		//(void*) 0x10002000,
		//(void*) 0x10001000
	};
	for (int i = 0; i < sizeof(devices) / sizeof(devices[0]); i++) {
		print("Address: ");
		print_hex((uintptr_t) devices[i]);
		print("\n");

		uint32_t magic = devices[i]->magic;
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
		print_hex(devices[i]->version);
		print("\n");

		print("Device ID: ");
		print_hex(devices[i]->device_id);
		print("\n");

		print("Vendor ID: ");
		print_hex(devices[i]->vendor_id);
		print("\n");

		if (devices[i]->device_id == 0x12) {
			struct virtio_config_input* config = (void*) devices[i]->config;

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
		print("\n");
	}
}
