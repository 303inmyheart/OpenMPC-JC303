#pragma once
namespace jc303drive {
struct ModelData { const char* name; const char* file; const char* json; int inputs; };
extern const ModelData models[];
extern const int modelCount;
}
