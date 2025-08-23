include_directories(
        ${PROJECT_UTIL_SRC_BASE}/htracer
        ${PROJECT_UTIL_SRC_BASE}/htracer/common
        ${PROJECT_UTIL_SRC_BASE}/htracer/manager
        ${PROJECT_UTIL_SRC_BASE}/htracer/service
)

aux_source_directory(. HTRACER_SRCS)
aux_source_directory(common HTRACER_SRCS)
aux_source_directory(manager HTRACER_SRCS)
aux_source_directory(service HTRACER_SRCS)

add_library(htracer_object OBJECT ${HTRACER_SRCS})
target_compile_options(htracer_object PRIVATE -fPIC)