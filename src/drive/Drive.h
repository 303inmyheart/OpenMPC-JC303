#pragma once
#include <memory>
#include <iosfwd>

class GuitarDrive {
public:
    static constexpr int First=59, Enable=59, Model=60, Level=61, Mix=62, Status=63;
    struct Settings { float enabled=0; int model=0; float level=.15f; float mix=.35f; };
    GuitarDrive();
    ~GuitarDrive();
    GuitarDrive(const GuitarDrive&)=delete;
    GuitarDrive& operator=(const GuitarDrive&)=delete;
    void set(int param,float value);
    float get(int param) const;
    int display(int param,char* out,int size) const;
    void beginRender();
    float process(float dry) noexcept;
    void save(std::ostream&) const;
    static bool parse(std::istream&,Settings&);
    void load(const Settings&);
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
