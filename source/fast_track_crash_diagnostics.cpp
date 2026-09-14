#include <cstdint>

#if (defined(MKW_LOCAL_FAST_TRACK) && MKW_LOCAL_FAST_TRACK) || \
    (defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK)
#define MKW_FAST_TRACK_DIAGNOSTICS 1
#include "abi_bridge.h"
#include "guest_flat_memory.h"
#include <switch.h>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#else
#define MKW_FAST_TRACK_DIAGNOSTICS 0
struct CpuContext;
#endif

#if MKW_FAST_TRACK_DIAGNOSTICS
namespace {

constexpr const char* kDispatchPath =
    "sdmc:/switch/WiiCompiled-Switch/fast-track-dispatch-blocker.txt";
constexpr const char* kExceptionPath =
    "sdmc:/switch/WiiCompiled-Switch/fast-track-exception.txt";
constexpr const char* kHeartbeatPath =
    "sdmc:/switch/WiiCompiled-Switch/fast-track-heartbeat.txt";
constexpr const char* kMainReachedPath =
    "sdmc:/switch/WiiCompiled-Switch/fast-track-main-reached.txt";
constexpr const char* kPostMainDispatchPath =
    "sdmc:/switch/WiiCompiled-Switch/fast-track-post-main-dispatch.txt";
constexpr const char* kPostMainTracePath =
    "sdmc:/switch/WiiCompiled-Switch/fast-track-post-main-trace.txt";
constexpr const char* kLegacyPostMainTracePath =
    "sdmc:/switch/WiiCompiled-Switch/fast-track-post-main-last-dispatch.txt";
constexpr std::uint32_t kPalMainAddress = 0x8000B6B0u;
constexpr std::size_t kDensePostMainTraceEntries = 48u;
constexpr std::size_t kMaxPostMainTraceEntries = 64u;
constexpr std::uint64_t kPostMainTraceFlushStride = 8u;
constexpr std::size_t kPostMainTraceStageSize = 40u;
constexpr std::size_t kPostMainTraceBufferSize = 16u * 1024u;

struct PostMainTraceEntry {
    std::uint64_t dispatch_count;
    std::uint64_t post_main_dispatch;
    std::uint32_t target;
    std::uint32_t guest_pc;
    std::uint32_t r1;
    std::uint32_t r2;
    std::uint32_t r3;
    std::uint32_t r13;
    char stage[kPostMainTraceStageSize];
};

const char* volatile g_fast_track_stage = "PROCESS_START";
bool g_liveness_files_reset = false;
bool g_main_reached = false;
bool g_post_main_dispatch_recorded = false;
std::uint64_t g_dispatch_count = 0u;
std::uint64_t g_post_main_dispatch_count = 0u;
std::uint64_t g_last_heartbeat_tick = 0u;
PostMainTraceEntry g_post_main_trace[kMaxPostMainTraceEntries]{};
std::size_t g_post_main_trace_size = 0u;
char g_post_main_trace_buffer[kPostMainTraceBufferSize]{};

const char* post_main_phase_name(std::uint32_t target) noexcept {
    switch (target) {
    case 0x80008EF0u:
        return "System::RKSystem::main";
    case 0x80008FB4u:
        return "EGG::BaseSystem::initialize";
    case 0x80009194u:
        return "System::RKSystem::initialize";
    case 0x8000951Cu:
        return "System::RKSystem::run";
    case 0x80243D18u:
        return "EGG::Video::initialize";
    case 0x80243D6Cu:
        return "EGG::Video::configure";
    default:
        return "-";
    }
}

bool is_durable_post_main_phase_target(std::uint32_t target) noexcept {
    switch (target) {
    // Keep this list aligned with the pinned WiiCompiled shard emitter's
    // runtime-toggle diagnostics / phase-tracing cold path. RKSystem::run is
    // included as an explicit application milestone even though it is not in
    // the generic-direct-call keep-list quoted by the pin.
    case 0x80008EF0u:
    case 0x80008FB4u:
    case 0x80009194u:
    case 0x8000951Cu:
    case 0x80243D18u:
    case 0x80243D6Cu:
    case 0x808897F0u:
    case 0x802226D8u:
    case 0x805C3218u:
    case 0x805E7460u:
    case 0x8063C470u:
    case 0x8063C4D4u:
    case 0x8063C560u:
    case 0x8063C714u:
    case 0x80198CA8u:
    case 0x80199038u:
    case 0x801992A8u:
    case 0x801998A4u:
    case 0x80226C78u:
    case 0x80226EBCu:
    case 0x80229814u:
    case 0x80229C5Cu:
    case 0x80229DCCu:
    case 0x80229DD8u:
    case 0x801A7424u:
    case 0x80672CC8u:
        return true;
    default:
        return false;
    }
}

std::size_t append_formatted(
    char* buffer,
    std::size_t capacity,
    std::size_t used,
    const char* format,
    ...) noexcept {
    if (!buffer || capacity == 0u || used >= capacity - 1u) {
        return used;
    }

    va_list args;
    va_start(args, format);
    const int n = std::vsnprintf(buffer + used, capacity - used, format, args);
    va_end(args);
    if (n <= 0) {
        return used;
    }

    const std::size_t appended = static_cast<std::size_t>(n);
    if (appended >= capacity - used) {
        return capacity - 1u;
    }
    return used + appended;
}

std::size_t format_post_main_trace(
    char* buffer,
    std::size_t capacity,
    const PostMainTraceEntry* entries,
    std::size_t count) noexcept {
    if (!buffer || capacity == 0u || (!entries && count != 0u)) {
        return 0u;
    }

    buffer[0] = '\0';
    std::size_t used = 0u;
    used = append_formatted(
        buffer,
        capacity,
        used,
        "WiiCompiled-Switch bounded post-main translated trace\n"
        "=====================================================\n"
        "entries               : %zu\n"
        "dense dispatch limit  : %zu\n"
        "total entry capacity  : %zu\n\n",
        count,
        kDensePostMainTraceEntries,
        kMaxPostMainTraceEntries);

    for (std::size_t i = 0u; i < count && used < capacity - 1u; ++i) {
        const PostMainTraceEntry& entry = entries[i];
        used = append_formatted(
            buffer,
            capacity,
            used,
            "[%02zu] post-main=%llu dispatch=%llu target=0x%08x pc=0x%08x "
            "r1=0x%08x r2=0x%08x r3=0x%08x r13=0x%08x stage=%s phase=%s\n",
            i + 1u,
            static_cast<unsigned long long>(entry.post_main_dispatch),
            static_cast<unsigned long long>(entry.dispatch_count),
            entry.target,
            entry.guest_pc,
            entry.r1,
            entry.r2,
            entry.r3,
            entry.r13,
            entry.stage,
            post_main_phase_name(entry.target));
    }

    return used;
}

void write_atomicish(const char* path, const char* data, std::size_t size) noexcept {
    const int fd = ::open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        return;
    }

    std::size_t written = 0;
    while (written < size) {
        const ssize_t rc = ::write(fd, data + written, size - written);
        if (rc <= 0) {
            break;
        }
        written += static_cast<std::size_t>(rc);
    }
    ::fsync(fd);
    ::close(fd);
}

void reset_liveness_files_once() noexcept {
    if (g_liveness_files_reset) {
        return;
    }
    g_liveness_files_reset = true;
    ::unlink(kDispatchPath);
    ::unlink(kExceptionPath);
    ::unlink(kHeartbeatPath);
    ::unlink(kMainReachedPath);
    ::unlink(kPostMainDispatchPath);
    ::unlink(kPostMainTracePath);
    ::unlink(kLegacyPostMainTracePath);
}

void write_liveness_record(
    const char* path,
    const char* title,
    std::uint32_t target,
    CpuContext* cpu) noexcept {
    char buffer[1024];
    const std::uint32_t guest_pc = cpu ? cpu->pc : 0u;
    const std::uint32_t r1 = cpu ? cpu->gpr[1] : 0u;
    const std::uint32_t r2 = cpu ? cpu->gpr[2] : 0u;
    const std::uint32_t r3 = cpu ? cpu->gpr[3] : 0u;
    const std::uint32_t r13 = cpu ? cpu->gpr[13] : 0u;

    const int n = std::snprintf(
        buffer,
        sizeof(buffer),
        "%s\n"
        "========================================\n"
        "dispatch count        : %llu\n"
        "post-main dispatch    : %llu\n"
        "last target           : 0x%08x\n"
        "guest pc              : 0x%08x\n"
        "r1                    : 0x%08x\n"
        "r2                    : 0x%08x\n"
        "r3                    : 0x%08x\n"
        "r13                   : 0x%08x\n"
        "fast-track stage      : %s\n"
        "PAL main              : 0x%08x\n"
        "main reached          : %s\n",
        title,
        static_cast<unsigned long long>(g_dispatch_count),
        static_cast<unsigned long long>(g_post_main_dispatch_count),
        target,
        guest_pc,
        r1,
        r2,
        r3,
        r13,
        g_fast_track_stage,
        kPalMainAddress,
        g_main_reached ? "YES" : "NO");
    if (n <= 0) {
        return;
    }

    const std::size_t size = static_cast<std::size_t>(n) < sizeof(buffer)
        ? static_cast<std::size_t>(n)
        : sizeof(buffer) - 1u;
    write_atomicish(path, buffer, size);
}

bool record_post_main_trace_entry(
    std::uint32_t target,
    CpuContext* cpu,
    bool phase_target) noexcept {
    if (g_post_main_trace_size >= kMaxPostMainTraceEntries) {
        return false;
    }

    const bool dense_dispatch =
        g_post_main_dispatch_count <= kDensePostMainTraceEntries;
    if (!dense_dispatch && !phase_target) {
        return false;
    }

    PostMainTraceEntry& entry = g_post_main_trace[g_post_main_trace_size++];
    entry.dispatch_count = g_dispatch_count;
    entry.post_main_dispatch = g_post_main_dispatch_count;
    entry.target = target;
    entry.guest_pc = cpu ? cpu->pc : 0u;
    entry.r1 = cpu ? cpu->gpr[1] : 0u;
    entry.r2 = cpu ? cpu->gpr[2] : 0u;
    entry.r3 = cpu ? cpu->gpr[3] : 0u;
    entry.r13 = cpu ? cpu->gpr[13] : 0u;
    const char* stage = g_fast_track_stage ? g_fast_track_stage : "<null>";
    std::snprintf(entry.stage, sizeof(entry.stage), "%s", stage);
    return true;
}

void flush_post_main_trace() noexcept {
    const std::size_t size = format_post_main_trace(
        g_post_main_trace_buffer,
        sizeof(g_post_main_trace_buffer),
        g_post_main_trace,
        g_post_main_trace_size);
    if (size != 0u) {
        write_atomicish(kPostMainTracePath, g_post_main_trace_buffer, size);
    }
}

} // namespace
#endif

#if MKW_FAST_TRACK_DIAGNOSTICS && \
    defined(MKW_SYNTHETIC_FAST_TRACK) && MKW_SYNTHETIC_FAST_TRACK
extern "C" bool mkw_switch_post_main_trace_synthetic_probe() noexcept {
    PostMainTraceEntry entries[2]{};
    entries[0].dispatch_count = 606u;
    entries[0].post_main_dispatch = 1u;
    entries[0].target = 0x80008EF0u;
    entries[0].guest_pc = 0x800060A4u;
    entries[0].r1 = 0x80399178u;
    entries[0].r2 = 0x8038EFA0u;
    entries[0].r3 = 0u;
    entries[0].r13 = 0x8038CC00u;
    std::snprintf(entries[0].stage, sizeof(entries[0].stage), "%s", "GUEST_POST_MAIN_ACTIVE");

    entries[1] = entries[0];
    entries[1].dispatch_count = 607u;
    entries[1].post_main_dispatch = 2u;
    entries[1].target = 0x80243D18u;

    char buffer[2048];
    const std::size_t size = format_post_main_trace(
        buffer,
        sizeof(buffer),
        entries,
        2u);
    return size != 0u &&
           std::strstr(buffer, "post-main=1 dispatch=606 target=0x80008ef0") != nullptr &&
           std::strstr(buffer, "phase=System::RKSystem::main") != nullptr &&
           std::strstr(buffer, "phase=EGG::Video::initialize") != nullptr;
}
#endif

extern "C" void mkw_switch_set_fast_track_stage(const char* stage) noexcept {
#if MKW_FAST_TRACK_DIAGNOSTICS
    g_fast_track_stage = stage ? stage : "<null>";
#else
    (void)stage;
#endif
}

extern "C" void mkw_switch_note_translated_dispatch(
    std::uint32_t target,
    CpuContext* cpu) noexcept {
#if MKW_FAST_TRACK_DIAGNOSTICS
    reset_liveness_files_once();
    ++g_dispatch_count;

    if (target == kPalMainAddress && !g_main_reached) {
        g_main_reached = true;
        g_fast_track_stage = "GUEST_MAIN_REACHED";
        write_liveness_record(
            kMainReachedPath,
            "WiiCompiled-Switch PAL main reached",
            target,
            cpu);
    }

    const bool post_main_dispatch =
        g_main_reached && target != kPalMainAddress;
    if (post_main_dispatch) {
        ++g_post_main_dispatch_count;
    }

    const bool first_post_main_dispatch =
        post_main_dispatch && !g_post_main_dispatch_recorded;
    if (first_post_main_dispatch) {
        g_post_main_dispatch_recorded = true;
        g_fast_track_stage = "GUEST_POST_MAIN_ACTIVE";
        write_liveness_record(
            kPostMainDispatchPath,
            "WiiCompiled-Switch first post-main translated dispatch",
            target,
            cpu);
    }

    if (post_main_dispatch) {
        const bool phase_target = is_durable_post_main_phase_target(target);
        const bool trace_entry_recorded =
            record_post_main_trace_entry(target, cpu, phase_target);
        const bool trace_flush_due =
            trace_entry_recorded &&
            (phase_target || g_post_main_trace_size == 1u ||
             g_post_main_trace_size == kDensePostMainTraceEntries ||
             g_post_main_trace_size == kMaxPostMainTraceEntries ||
             (g_post_main_dispatch_count <= kDensePostMainTraceEntries &&
              g_post_main_dispatch_count % kPostMainTraceFlushStride == 0u));
        if (trace_flush_due) {
            flush_post_main_trace();
        }
    }

    const std::uint64_t now = armGetSystemTick();
    const std::uint64_t frequency = armGetSystemTickFreq();
    const bool heartbeat_due =
        g_last_heartbeat_tick == 0u || frequency == 0u ||
        now - g_last_heartbeat_tick >= frequency;
    if (heartbeat_due || target == kPalMainAddress || first_post_main_dispatch) {
        g_last_heartbeat_tick = now;
        write_liveness_record(
            kHeartbeatPath,
            "WiiCompiled-Switch translated liveness heartbeat",
            target,
            cpu);
    }
#else
    (void)target;
    (void)cpu;
#endif
}

extern "C" void mkw_switch_report_unsupported_translated_dispatch(
    const char* kind,
    std::uint32_t target,
    CpuContext* cpu) noexcept {
#if MKW_FAST_TRACK_DIAGNOSTICS
    char buffer[1024];
    const std::uint32_t guest_pc = cpu ? cpu->pc : 0u;
    const std::uint32_t r1 = cpu ? cpu->gpr[1] : 0u;
    const std::uint32_t r2 = cpu ? cpu->gpr[2] : 0u;
    const std::uint32_t r3 = cpu ? cpu->gpr[3] : 0u;
    const std::uint32_t r13 = cpu ? cpu->gpr[13] : 0u;

    const int n = std::snprintf(
        buffer,
        sizeof(buffer),
        "WiiCompiled-Switch unsupported translated dispatch\n"
        "=================================================\n"
        "kind                  : %s\n"
        "target                : 0x%08x\n"
        "guest pc              : 0x%08x\n"
        "r1                    : 0x%08x\n"
        "r2                    : 0x%08x\n"
        "r3                    : 0x%08x\n"
        "r13                   : 0x%08x\n"
        "fast-track stage      : %s\n"
        "action                : abort after durable blocker record\n",
        kind ? kind : "UNKNOWN",
        target,
        guest_pc,
        r1,
        r2,
        r3,
        r13,
        g_fast_track_stage);
    if (n > 0) {
        const std::size_t size = static_cast<std::size_t>(n) < sizeof(buffer)
            ? static_cast<std::size_t>(n)
            : sizeof(buffer) - 1u;
        write_atomicish(kDispatchPath, buffer, size);
    }
#else
    (void)kind;
    (void)target;
    (void)cpu;
#endif
}

#if MKW_FAST_TRACK_DIAGNOSTICS
extern "C" {
// libnx defaults to a very small exception stack. Crash reporting uses only
// fixed buffers/no heap, but give it enough room for libc formatting and FS I/O.
alignas(16) u8 __nx_exception_stack[0x4000];
u64 __nx_exception_stack_size = sizeof(__nx_exception_stack);
}

extern "C" void __libnx_exception_handler(ThreadExceptionDump* ctx) {
    if (!ctx) {
        return;
    }

    CpuContext* tls_guest = TryGetCpuContext();
    CpuContext* guest = tls_guest;
    bool persistent_fallback = false;
    if (!guest) {
        guest = &GetPersistentCpuContext();
        persistent_fallback = true;
    }

    const std::uint32_t guest_pc = guest ? guest->pc : 0u;
    const std::uint32_t guest_r1 = guest ? guest->gpr[1] : 0u;
    const std::uint32_t guest_r2 = guest ? guest->gpr[2] : 0u;
    const std::uint32_t guest_r3 = guest ? guest->gpr[3] : 0u;
    const std::uint32_t guest_r13 = guest ? guest->gpr[13] : 0u;

    const std::uintptr_t guest_base =
        reinterpret_cast<std::uintptr_t>(GuestFlat::Base());
    const std::uintptr_t far = static_cast<std::uintptr_t>(ctx->far.x);
    const bool far_in_guest_window =
        guest_base != 0u && far >= guest_base &&
        static_cast<std::uint64_t>(far - guest_base) < GuestFlat::kGuestSpaceSize;
    const std::uint32_t derived_guest_address =
        far_in_guest_window ? static_cast<std::uint32_t>(far - guest_base) : 0u;

    char buffer[4096];
    const int n = std::snprintf(
        buffer,
        sizeof(buffer),
        "WiiCompiled-Switch libnx exception\n"
        "==================================\n"
        "error desc            : 0x%08x\n"
        "fast-track stage      : %s\n"
        "aarch64 pc            : 0x%016llx\n"
        "aarch64 lr            : 0x%016llx\n"
        "aarch64 sp            : 0x%016llx\n"
        "fault address (FAR)   : 0x%016llx\n"
        "ESR                   : 0x%08x\n"
        "x0                    : 0x%016llx\n"
        "x1                    : 0x%016llx\n"
        "x2                    : 0x%016llx\n"
        "x3                    : 0x%016llx\n"
        "x4                    : 0x%016llx\n"
        "x5                    : 0x%016llx\n"
        "x6                    : 0x%016llx\n"
        "x7                    : 0x%016llx\n"
        "x8                    : 0x%016llx\n"
        "guest context active  : %s\n"
        "guest context source  : %s\n"
        "guest pc              : 0x%08x\n"
        "guest r1              : 0x%08x\n"
        "guest r2              : 0x%08x\n"
        "guest r3              : 0x%08x\n"
        "guest r13             : 0x%08x\n"
        "guest flat base       : 0x%016llx\n"
        "FAR in guest window   : %s\n"
        "derived guest address : 0x%08x\n"
        "note                  : process will still terminate after this handler\n",
        ctx->error_desc,
        g_fast_track_stage,
        static_cast<unsigned long long>(ctx->pc.x),
        static_cast<unsigned long long>(ctx->lr.x),
        static_cast<unsigned long long>(ctx->sp.x),
        static_cast<unsigned long long>(ctx->far.x),
        ctx->esr,
        static_cast<unsigned long long>(ctx->cpu_gprs[0].x),
        static_cast<unsigned long long>(ctx->cpu_gprs[1].x),
        static_cast<unsigned long long>(ctx->cpu_gprs[2].x),
        static_cast<unsigned long long>(ctx->cpu_gprs[3].x),
        static_cast<unsigned long long>(ctx->cpu_gprs[4].x),
        static_cast<unsigned long long>(ctx->cpu_gprs[5].x),
        static_cast<unsigned long long>(ctx->cpu_gprs[6].x),
        static_cast<unsigned long long>(ctx->cpu_gprs[7].x),
        static_cast<unsigned long long>(ctx->cpu_gprs[8].x),
        tls_guest ? "YES" : "NO",
        persistent_fallback ? "PERSISTENT_FALLBACK" : "TLS",
        guest_pc,
        guest_r1,
        guest_r2,
        guest_r3,
        guest_r13,
        static_cast<unsigned long long>(guest_base),
        far_in_guest_window ? "YES" : "NO",
        derived_guest_address);

    if (n > 0) {
        const std::size_t size = static_cast<std::size_t>(n) < sizeof(buffer)
            ? static_cast<std::size_t>(n)
            : sizeof(buffer) - 1u;
        write_atomicish(kExceptionPath, buffer, size);
    }
}
#endif
