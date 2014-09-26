#pragma once

#include <boost/noncopyable.hpp>

#include <fstream>
#include <memory>
#include <string>

class TempFile : public boost::noncopyable {
public:
    typedef std::unique_ptr<TempFile> ptr;

    static const char* sys_tmpdir();
    enum Mode {
        LEAVE,
        CLEANUP,
        ANON
    };

    explicit TempFile(Mode mode);
    TempFile(const std::string& tmpl, Mode mode);

    static ptr create(Mode mode);
    static ptr create(const std::string& tmpl, Mode mode);

    virtual ~TempFile();
    const std::string& path() const;
    std::fstream& stream();

private:
    void _mkstemp();

private:
    std::fstream _stream;
    std::string _path;
    Mode _mode;
};

class TempDir : public boost::noncopyable {
public:
    typedef std::unique_ptr<TempDir> ptr;

    enum Mode {
        LEAVE,
        CLEANUP
    };

    explicit TempDir(Mode mode);
    TempDir(const std::string& tmpl, Mode mode);

    static ptr create(Mode mode);
    static ptr create(const std::string& tmpl, Mode mode);
    virtual ~TempDir();

    const std::string& path() const;
    TempFile::ptr tempFile(TempFile::Mode mode) const;

private:
    void _mkdtemp();

private:
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
