#!/bin/bash
set -e

ENC_DIR="$PWD"

# Copy test data
rm -rf data
mkdir data

rm -rf *.tgz

# Download model
wget https://storage.googleapis.com/download.tensorflow.org/models/mobilenet_v1_2018_02_22/mobilenet_v1_1.0_224.tgz
tar xzv -f mobilenet_v1_1.0_224.tgz -C data

# Download labels
wget https://storage.googleapis.com/download.tensorflow.org/models/mobilenet_v1_1.0_224_frozen.tgz
tar xzv -f mobilenet_v1_1.0_224_frozen.tgz  -C data
cp data/mobilenet_v1_1.0_224/labels.txt data/

# Clone tensorflow
rm -rf tensorflow
git clone https://github.com/tensorflow/tensorflow --branch v2.4.1 --depth 1

# Copy test image
cp tensorflow/tensorflow/lite/examples/label_image/testdata/grace_hopper.bmp data/

rm -rf "$ENC_DIR/../data"
mv data "$ENC_DIR/../"

# Configure tensorflow-lite
rm -rf build
mkdir build
cd build || exit 1
export CFLAGS="-DEIGEN_NO_CPUID"
export CXXFLAGS="$CFLAGS"
cmake ../tensorflow/tensorflow/lite -DCMAKE_BUILD_TYPE=Debug

# Build ruy
(
    mkdir build-ruy
    cd build-ruy || exit 1
    cmake ../ruy -DCMAKE_BUILD_TYPE=Debug
    cd ruy
    make -j 16
    ar -M  <<EOF 
  create libruy.a
  addlib libruy_allocator.a
  addlib libruy_cpuinfo.a
  addlib libruy_pack_avx512.a
  addlib libruy_thread_pool.a
  addlib libruy_apply_multiplier.a
  addlib libruy_ctx.a
  addlib libruy_kernel_avx2_fma.a
  addlib libruy_pack_avx.a
  addlib libruy_trmul.a
  addlib libruy_blocking_counter.a
  addlib libruy_frontend.a
  addlib libruy_kernel_avx512.a
  addlib libruy_pmu.a
  addlib libruy_tune.a
  addlib libruy_block_map.a
  addlib libruy_have_built_path_for_avx2_fma.a
  addlib libruy_kernel_avx.a
  addlib libruy_prepacked_cache.a
  addlib libruy_wait.a
  addlib libruy_context.a
  addlib libruy_have_built_path_for_avx512.a
  addlib libruy_pack_arm.a
  addlib libruy_prepare_packed_matrices.a  
  addlib libruy_context_get_ctx.a
  addlib libruy_have_built_path_for_avx.a
  addlib libruy_pack_avx2_fma.a
  addlib libruy_system_aligned_alloc.a
  save
  end
EOF
    cp libruy.a "$ENC_DIR/"
)

make -j 16


# Copy libraries
cp clog/libclog.a "$ENC_DIR"
cp cpuinfo/libcpuinfo.a "$ENC_DIR"
cp libtensorflow-lite.a "$ENC_DIR"
touch "$ENC_DIR/empty.c"
