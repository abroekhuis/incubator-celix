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
 * capability_test.cpp
 *
 *  \date       Feb 11, 2013
 *  \author     <a href="mailto:celix-dev@incubator.apache.org">Apache Celix Project Team</a>
 *  \copyright  Apache License, Version 2.0
 */
#include <stdlib.h>
#include <stdio.h>

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "capability_private.h"
#include "attribute.h"
}

int main(int argc, char** argv) {
	RUN_ALL_TESTS(argc, argv);
	return 0;
}

TEST_GROUP(capability) {
	apr_pool_t *pool;

	void setup(void) {
		apr_initialize();
		apr_pool_create(&pool, NULL);
	}

	void teardown() {
		apr_pool_destroy(pool);
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(capability, create) {
	module_pt module = (module_pt) 0x10;
	hash_map_pt directives = hashMap_create(NULL, NULL, NULL, NULL);
	hash_map_pt attributes = hashMap_create(NULL, NULL, NULL, NULL);

	version_pt emptyVersion = (version_pt) 0x20; //apr_palloc(pool, sizeof(*version));
	attribute_pt attribute = (attribute_pt) 0x30;
	char key[] = "key";

	mock().expectOneCall("version_createEmptyVersion")
			.withParameter("pool", pool)
			.andOutputParameter("version", emptyVersion)
			.andReturnValue(CELIX_SUCCESS);
	mock().expectOneCall("attribute_getValue")
			// .withParameter("attribute", (void *) 0x0)
			// .andOutputParameter("value", (char *) 0x0)
			.andReturnValue(CELIX_SUCCESS);

	capability_pt capability = NULL;
	celix_status_t status = capability_create(pool, module, directives, attributes, &capability);
}

TEST(capability, getServiceName) {
	capability_pt capability = (capability_pt) apr_palloc(pool, sizeof(*capability));
	char serviceName[] = "service";
	capability->serviceName = serviceName;

	char *actual = NULL;
	capability_getServiceName(capability, &actual);
	STRCMP_EQUAL(serviceName, actual);
}

TEST(capability, getVersion) {
	capability_pt capability = (capability_pt) apr_palloc(pool, sizeof(*capability));
	version_pt version = (version_pt) 0x10;
	capability->version = version;

	version_pt actual = NULL;
	capability_getVersion(capability, &actual);
	POINTERS_EQUAL(version, actual);
}

TEST(capability, getModule) {
	capability_pt capability = (capability_pt) apr_palloc(pool, sizeof(*capability));
	module_pt module = (module_pt) 0x10;
	capability->module = module;

	module_pt actual = NULL;
	capability_getModule(capability, &actual);
	POINTERS_EQUAL(module, actual);
}
