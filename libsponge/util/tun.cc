#include "tun.hh"
#include "util.hh"
#include <fcntl.h>
#include <stdexcept>
#include <cstring>

#ifdef __linux__
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
static constexpr const char *CLONEDEV = "/dev/net/tun";

using namespace std;

TunTapFD::TunTapFD(const string &devname, const bool is_tun)
    : FileDescriptor(SystemCall("open", open(CLONEDEV, O_RDWR))) {
    struct ifreq tun_req {};
    tun_req.ifr_flags = (is_tun ? IFF_TUN : IFF_TAP) | IFF_NO_PI;
    strncpy(static_cast<char *>(tun_req.ifr_name), devname.data(), IFNAMSIZ - 1);
    tun_req.ifr_name[IFNAMSIZ - 1] = '\0';
    SystemCall("ioctl", ioctl(fd_num(), TUNSETIFF, static_cast<void *>(&tun_req)));
}
#else
// macOS stub: TUN/TAP not supported on macOS without third-party kernel extensions
// Use the CS144 Linux VM for labs requiring TUN devices (Lab 4+)
#include <unistd.h>
#include <cerrno>

using namespace std;

TunTapFD::TunTapFD(const string &devname, const bool is_tun)
    : FileDescriptor(SystemCall("open", open("/dev/null", O_RDWR))) {}
#endif
