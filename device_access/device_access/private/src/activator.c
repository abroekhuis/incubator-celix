/**
 *Licensed to the Apache Software Foundation (ASF) under one
 *or more contributor license agreements.  See the NOTICE file
 *distributed with this work for additional information
 *regarding copyright ownership.  The ASF licenses this file
 *to you under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in compliance
 *with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing,
 *software distributed under the License is distributed on an
 *"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 *specific language governing permissions and limitations
 *under the License.
 */
/*
 * activator.c
 *
 *  \date       Jun 20, 2011
 *  \author    	<a href="mailto:celix-dev@incubator.apache.org">Apache Celix Project Team</a>
 *  \copyright	Apache License, Version 2.0
 */
#include <stdlib.h>
#include <apr_strings.h>

#include <bundle_activator.h>
#include <bundle_context.h>
#include <celixbool.h>
#include <service_tracker.h>

#include "driver_locator.h"
#include "device_manager.h"

struct device_manager_bundle_instance {
	bundle_context_pt context;
	apr_pool_t *pool;
	device_manager_pt deviceManager;
	service_tracker_pt driverLocatorTracker;
	service_tracker_pt driverTracker;
	service_tracker_pt deviceTracker;
};

typedef struct device_manager_bundle_instance *device_manager_bundle_instance_pt;

static celix_status_t deviceManagerBundle_createDriverLocatorTracker(device_manager_bundle_instance_pt bundleData);
static celix_status_t deviceManagerBundle_createDriverTracker(device_manager_bundle_instance_pt bundleData);
static celix_status_t deviceManagerBundle_createDeviceTracker(device_manager_bundle_instance_pt bundleData);

celix_status_t addingService_dummy_func(void * handle, service_reference_pt reference, void **service) {
	celix_status_t status = CELIX_SUCCESS;
	device_manager_pt dm = handle;
	bundle_context_pt context = NULL;
	status = deviceManager_getBundleContext(dm, &context);
	if (status == CELIX_SUCCESS) {
		status = bundleContext_getService(context, reference, service);
	}
	return status;
}

celix_status_t bundleActivator_create(bundle_context_pt context, void **userData) {
	celix_status_t status = CELIX_SUCCESS;
	apr_pool_t *pool;
	status = bundleContext_getMemoryPool(context, &pool);
	if (status == CELIX_SUCCESS) {
		device_manager_bundle_instance_pt bi = apr_palloc(pool, sizeof(struct device_manager_bundle_instance));
		if (bi == NULL) {
			status = CELIX_ENOMEM;
		} else {
			(*userData) = bi;
			bi->context = context;
			bi->pool = pool;
			status = deviceManager_create(pool, context, &bi->deviceManager);
		}
	}
	return status;
}

celix_status_t bundleActivator_start(void * userData, bundle_context_pt context) {
	celix_status_t status = CELIX_SUCCESS;
	device_manager_bundle_instance_pt bundleData = userData;
	apr_pool_t *pool;
	status = bundleContext_getMemoryPool(context, &pool);
	if (status == CELIX_SUCCESS) {
		status = deviceManagerBundle_createDriverLocatorTracker(bundleData);
		if (status == CELIX_SUCCESS) {
			status = deviceManagerBundle_createDriverTracker(bundleData);
			if (status == CELIX_SUCCESS) {
					status = deviceManagerBundle_createDeviceTracker(bundleData);
					if (status == CELIX_SUCCESS) {
						status = serviceTracker_open(bundleData->driverLocatorTracker);
						if (status == CELIX_SUCCESS) {
							status = serviceTracker_open(bundleData->driverTracker);
							if (status == CELIX_SUCCESS) {
								status = serviceTracker_open(bundleData->deviceTracker);
							}
						}
					}
			}
		}
	}

	if (status != CELIX_SUCCESS) {
		printf("DEVICE_MANAGER: Error while starting bundle got error num %i\n", status);
	}

	printf("DEVICE_MANAGER: Started\n");
	return status;
}

static celix_status_t deviceManagerBundle_createDriverLocatorTracker(device_manager_bundle_instance_pt bundleData) {
	celix_status_t status = CELIX_SUCCESS;

	service_tracker_customizer_pt customizer = NULL;

	status = serviceTrackerCustomizer_create(bundleData->pool, bundleData->deviceManager, addingService_dummy_func,
			deviceManager_locatorAdded, deviceManager_locatorModified, deviceManager_locatorRemoved, &customizer);

	if (status == CELIX_SUCCESS) {
		service_tracker_pt tracker = NULL;
		status = serviceTracker_create(bundleData->pool, bundleData->context, "driver_locator", customizer, &tracker);
		if (status == CELIX_SUCCESS) {
			bundleData->driverLocatorTracker=tracker;
		}
	} else {
		status = CELIX_ENOMEM;
	}
	return status;
}

static celix_status_t deviceManagerBundle_createDriverTracker(device_manager_bundle_instance_pt bundleData) {
	celix_status_t status = CELIX_SUCCESS;

	service_tracker_customizer_pt customizer = NULL;

	status = serviceTrackerCustomizer_create(bundleData->pool, bundleData->deviceManager, addingService_dummy_func,
			deviceManager_driverAdded, deviceManager_driverModified, deviceManager_driverRemoved, &customizer);

	if (status == CELIX_SUCCESS) {
		service_tracker_pt tracker = NULL;
		status = serviceTracker_createWithFilter(bundleData->pool, bundleData->context, "(objectClass=driver)", customizer, &tracker);
		if (status == CELIX_SUCCESS) {
			bundleData->driverTracker=tracker;
		}
	} else {
		status = CELIX_ENOMEM;
	}
	return status;
}

static celix_status_t deviceManagerBundle_createDeviceTracker(device_manager_bundle_instance_pt bundleData) {
	celix_status_t status = CELIX_SUCCESS;

	service_tracker_customizer_pt customizer = NULL;

	status = serviceTrackerCustomizer_create(bundleData->pool, bundleData->deviceManager, addingService_dummy_func,
			deviceManager_deviceAdded, deviceManager_deviceModified, deviceManager_deviceRemoved, &customizer);

	if (status == CELIX_SUCCESS) {
		service_tracker_pt tracker = NULL;
		status = serviceTracker_createWithFilter(bundleData->pool, bundleData->context, "(|(objectClass=device)(DEVICE_CATEGORY=*))", customizer, &tracker);
		if (status == CELIX_SUCCESS) {
			bundleData->deviceTracker=tracker;
		}
	} else {
		status = CELIX_ENOMEM;
	}
	return status;
}

celix_status_t bundleActivator_stop(void * userData, bundle_context_pt context) {
	celix_status_t status = CELIX_SUCCESS;
	device_manager_bundle_instance_pt bundleData = userData;
//	status = serviceTracker_close(bundleData->driverLocatorTracker);
	if (status == CELIX_SUCCESS) {
		status = serviceTracker_close(bundleData->driverTracker);
		if (status == CELIX_SUCCESS) {
			status = serviceTracker_close(bundleData->deviceTracker);
		}
	}
	return status;
}

celix_status_t bundleActivator_destroy(void * userData, bundle_context_pt context) {
	celix_status_t status = CELIX_SUCCESS;
	device_manager_bundle_instance_pt bundleData = userData;
	deviceManager_destroy(bundleData->deviceManager);
	return CELIX_SUCCESS;
}
