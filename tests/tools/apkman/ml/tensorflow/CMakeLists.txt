# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

add_subdirectory(host)

if (BUILD_ENCLAVES)
  add_subdirectory(enc)
endif ()

add_enclave_test(
  tests/tools/apkman/tensorflow
  tensorflow_host
  tensorflow_enc
  --tflite_model
  ${CMAKE_CURRENT_BINARY_DIR}/data/mobilenet_v1_1.0_224.tflite
  --labels
  ${CMAKE_CURRENT_BINARY_DIR}/data/labels.txt
  --image
  ${CMAKE_CURRENT_BINARY_DIR}/data/grace_hopper.bmp)
