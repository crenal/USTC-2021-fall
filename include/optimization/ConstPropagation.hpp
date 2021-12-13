#ifndef CONSTPROPAGATION_HPP
#define CONSTPROPAGATION_HPP
#include "PassManager.hpp"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"
#include "Value.h"
#include "IRBuilder.h"
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>

class ConstPropagation : public Pass
{
private:
public:
    ConstPropagation(Module *m) : Pass(m) {}
    void run();
    void const_replace();
};

#endif