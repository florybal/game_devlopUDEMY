#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <dirent.h> // Para listar diretórios
#include <nvml.h>
#include <chrono>



nvmlDevice_t getNVMLDevice()
{
    nvmlReturn_t result;

    result = nvmlInit_v2();

    if (result != NVML_SUCCESS)
    {
        std::cerr << "Erro ao inicializar NVML: "
                  << nvmlErrorString(result)
                  << std::endl;

        return nullptr;
    }

    nvmlDevice_t device;

    result = nvmlDeviceGetHandleByIndex_v2(0, &device);

    if (result != NVML_SUCCESS)
    {
        std::cerr << "Erro ao obter GPU NVML: "
                  << nvmlErrorString(result)
                  << std::endl;

        return nullptr;
    }

    return device;
}


// --- MEMÓRIA (RAM e Virtual) ---
struct MemoryInfo {
    long long rss_kb;      // RAM Física (VmRSS)
    long long virtual_kb;  // Memória Virtual (VmSize)
};

MemoryInfo getProcessMemory() {
    MemoryInfo info = {0, 0};
    std::ifstream file("/proc/self/status"); // Use "/proc/[PID]/status" para outro processo
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            info.rss_kb = std::stoll(line.substr(6));
        } else if (line.substr(0, 7) == "VmSize:") {
            info.virtual_kb = std::stoll(line.substr(7));
        }
    }
    return info;
}

// --- CPU (Específica do Processo) ---
// Requer manter o último valor lido para calcular a diferença (delta)
struct CpuSnapshot {
    unsigned long long utime;
    unsigned long long stime;
    unsigned long long total_time; // Tempo total do sistema (para normalizar se quiser %)
};

CpuSnapshot getCpuSnapshot() {
    CpuSnapshot snap = {0, 0, 0};
    std::ifstream file("/proc/self/stat"); // Use "/proc/[PID]/stat" para outro processo
    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string temp;
        // Pula os primeiros 13 campos (pid, comm, state, ppid, etc.)
        for (int i = 0; i < 13; ++i) iss >> temp;
        
        unsigned long long utime, stime;
        iss >> utime >> stime; // Campos 14 e 15
        
        snap.utime = utime;
        snap.stime = stime;
    }
    
    // Precisamos também do tempo total do sistema para calcular a % real
    std::ifstream statFile("/proc/stat");
    if (statFile.is_open()) {
        std::string cpuLine;
        std::getline(statFile, cpuLine);
        std::istringstream iss(cpuLine);
        std::string cpu;
        unsigned long long user, nice, sys, idle, iowait, irq, softirq, steal;
        iss >> cpu >> user >> nice >> sys >> idle >> iowait >> irq >> softirq >> steal;
        snap.total_time = user + nice + sys + idle + iowait + irq + softirq + steal;
    }
    
    return snap;
}

double calculateCpuPercent(const CpuSnapshot& current, const CpuSnapshot& last) {
    unsigned long long totalProcessTime = (current.utime - last.utime) + (current.stime - last.stime);
    unsigned long long totalSystemTime = current.total_time - last.total_time;
    
    if (totalSystemTime == 0) return 0.0;
    
    // Resultado pode ser > 100% se usar múltiplos núcleos
    return (double)totalProcessTime / (double)totalSystemTime * 100.0;
}
