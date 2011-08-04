#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <boost/noncopyable.hpp>

class TempFile : public boost::noncopyable {
public:
    typedef std::shared_ptr<TempFile> ptr;

    static const char* sys_tmpdir();
    enum Mode {
        LEAVE,
        CLEANUP,
        ANON
    };

    static ptr create(Mode mode);
    static ptr create(const std::string& tmpl, Mode mode);

    virtual ~TempFile();
    const std::string& path() const;
    std::fstream& stream();

protected:
    explicit TempFile(Mode mode);
    TempFile(const std::string& tmpl, Mode mode);

    void _mkstemp();

protected:
    std::fstream _stream;
    std::string _path;
    Mode _mode;
};

class TempDir : public boost::noncopyable {
public:
    typedef std::shared_ptr<TempDir> ptr;

    enum Mode {
        LEAVE,
        CLEANUP
    };

    static ptr create(Mode mode);
    static ptr create(const std::string& tmpl, Mode mode);
    virtual ~TempDir();

    const std::string& path() const;
    TempFile::ptr tempFile(TempFile::Mode mode) const;

protected:
    explicit TempDir(Mode mode);
    TempDir(const std::string& tmpl, Mode mode);
    void _mkdtemp();

protected:
    std::string _path;
    Mode _mode;
};

inline const std::string& TempFile::path() const {
    return _path;
}

inline std::fstream& TempFile::stream() {
    return _stream;
}

inline const std::string& TempDir::path() const {
    return _path;
}
