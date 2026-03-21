#import <Flux/Platform/MemoryFootprint.hpp>
#import <Flux/Core/Log.hpp>

#import <mach/mach.h>
#import <cstdlib>

namespace flux {

void logMemoryFootprintIfRequested(const char* tag) {
    if (!std::getenv("FLUX_MEMORY_REPORT")) {
        return;
    }
    task_vm_info_data_t vmInfo{};
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    kern_return_t kr = task_info(mach_task_self(), TASK_VM_INFO, (task_info_t)&vmInfo, &count);
    if (kr != KERN_SUCCESS) {
        FLUX_LOG_WARN("[Memory] %s: task_info(TASK_VM_INFO) failed (%d)", tag ? tag : "", kr);
        return;
    }
    const uint64_t residentKb = static_cast<uint64_t>(vmInfo.resident_size) / 1024;
    const uint64_t footprintKb = static_cast<uint64_t>(vmInfo.phys_footprint) / 1024;
    FLUX_LOG_INFO("[Memory] %s: phys_footprint=%llu MiB resident=%llu MiB",
                  tag ? tag : "",
                  static_cast<unsigned long long>((footprintKb + 512) / 1024),
                  static_cast<unsigned long long>((residentKb + 512) / 1024));
}

} // namespace flux
