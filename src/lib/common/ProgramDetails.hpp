#pragma once

#include "version.h"

#include <sstream>
#include <string>
#include <boost/filesystem/path.hpp>

namespace {
    const char* installPrefix() {
        return __g_install_prefix;
    }

    const char* installedSharePath() {
        const char* share = getenv("JOINX_SHARE_PATH");
        if (share != NULL)
            return share;

        return __g_share_path;
    }

    std::string makeProgramVersionInfo(const std::string& progname) {
        using namespace std;
        ostringstream ss;
        ss << progname << " version " << __g_prog_version;
        if (!string(__g_commit_hash).empty())
            ss << " (commit " << __g_commit_hash << ")";

        if (!string(__g_build_type).empty())
            ss << " (" << __g_build_type << ")";

        ss << "\ninstall prefix: " << installPrefix() << "\n"
            << "share directory: " << installedSharePath();
        return ss.str();
    }
}
