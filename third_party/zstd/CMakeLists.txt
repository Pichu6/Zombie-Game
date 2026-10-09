# zstd 1.5.7 (BSD licence, third_party/zstd/LICENSE): the library only, from the official release tarball
# (sha256 eb33e51f49a15e023950cd7825ca74a4a2b43db8354825ac24fc1b7ee09e6fa3). Built single-threaded and without the
# x86-64 assembly decoder, so every compiler builds the same C (spec 06 rule 17, decision 0055 section 4).
enable_language(C)
file(GLOB ZS_ZSTD_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/common/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/compress/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/decompress/*.c)
add_library(zs_zstd STATIC ${ZS_ZSTD_SOURCES})
target_include_directories(zs_zstd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_compile_definitions(zs_zstd PRIVATE ZSTD_DISABLE_ASM=1)
