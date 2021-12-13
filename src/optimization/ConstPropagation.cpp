#include "ConstPropagation.hpp"
#include "logging.hpp"
std::set<BasicBlock*> visitedBB;//记录已经优化过的基本块
std::stack<BasicBlock*> blocks;//工作区
std::set<Instruction*> ins2del;//待删除语句
ConstantFP *cast_constantfp(Value *value)//判断是否是浮点数
{
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr)
    {
        return constant_fp_ptr;
    }
    else
    {
        return nullptr;
    }
}
ConstantInt *cast_constantint(Value *value)//判断是否是整数
{
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr)
    {
        return constant_int_ptr;
    }
    else
    {
        return nullptr;
    }
}
ConstantInt *compute_int(Instruction::OpID op,Value *value1,Value *value2,Module *module_)//整数运算
{

    int l_var = dynamic_cast<ConstantInt*>(value1)->get_value();
    int r_val = dynamic_cast<ConstantInt*>(value2)->get_value();
    int add_r=l_var + r_val,
        sub_r=l_var - r_val,
        mul_r=l_var * r_val,
        sdiv_r=(int)(l_var / r_val);
    if(op==Instruction::add)
        return ConstantInt::get(add_r, module_);
    else if(op==Instruction::sub)
        return ConstantInt::get(sub_r, module_);
    else if(op==Instruction::mul)
        return ConstantInt::get(mul_r, module_);
    else if(op==Instruction::sdiv)
        return ConstantInt::get(sdiv_r, module_);
}
ConstantFP *compute_fp(Instruction::OpID op,Value *value1,Value *value2,Module *module_)//浮点数运算
{
    float l_var = dynamic_cast<ConstantFP*>(value1)->get_value();
    float r_val = dynamic_cast<ConstantFP*>(value2)->get_value();
    float add_r=l_var + r_val,
          sub_r=l_var - r_val,
          mul_r=l_var * r_val,
          sdiv_r=(l_var / r_val);
    if(op==Instruction::fadd)
        return ConstantFP::get(add_r, module_);
    else if(op==Instruction::fsub)
        return ConstantFP::get(sub_r, module_);
    else if(op==Instruction::fmul)
        return ConstantFP::get(mul_r, module_);
    else if(op==Instruction::fdiv)
        return ConstantFP::get(sdiv_r, module_);
}

ConstantInt *conpute_i_compare(CmpInst::CmpOp op, Value* l_var, Value* r_var,  Module* module_)//整数比较
{
    int l_op = dynamic_cast<ConstantInt*>(l_var)->get_value();
    int r_op = dynamic_cast<ConstantInt*>(r_var)->get_value();
    bool result;
    switch (op)
    {
    case CmpInst::EQ:
        return ConstantInt::get((l_op==r_op), module_);
    case CmpInst::NE:
        return ConstantInt::get((l_op!=r_op), module_);
    case CmpInst::LE:
        return ConstantInt::get((l_op<=r_op), module_);
    case CmpInst::LT:
        return ConstantInt::get((l_op<r_op), module_);
    case CmpInst::GE:
        return ConstantInt::get((l_op>=r_op), module_);
    case CmpInst::GT:
        return ConstantInt::get((l_op>r_op), module_);
    default:
        return nullptr;
    }
}

ConstantInt *compute_f_compare(FCmpInst::CmpOp op, Value* l_var, Value* r_var,  Module* module_)//浮点数比较
{
    float l_op = dynamic_cast<ConstantFP*>(l_var)->get_value();
    float r_op = dynamic_cast<ConstantFP*>(r_var)->get_value();
    bool result;
    switch (op)
    {
    case CmpInst::EQ:
         return ConstantInt::get((l_op==r_op), module_);
    case CmpInst::NE:
         return ConstantInt::get((l_op!=r_op), module_);
    case CmpInst::LE:
         return ConstantInt::get(l_op<=r_op, module_);
    case CmpInst::LT:
         return ConstantInt::get((l_op<r_op), module_);
    case CmpInst::GE:
        return ConstantInt::get((l_op>=r_op), module_);
    case CmpInst::GT:
         return ConstantInt::get((l_op>r_op), module_);
    default:
        return nullptr;
    }
}
void ConstPropagation::const_replace()//常量替代与死代码删除
{
    while (!blocks.empty())
        {
            BasicBlock* bb = blocks.top();
            blocks.pop();
            if(!bb->empty())
            {
            for(auto instr: bb->get_instructions())
            {
                if(instr->is_si2fp())//类型转化
                {
                    if(cast_constantint(instr->get_operand(0)))
                    {
                        auto a_fp=dynamic_cast<ConstantInt*>(instr->get_operand(0));
                        instr->replace_all_use_with(ConstantFP::get(a_fp->get_value(), m_));
                        ins2del.insert(instr);
                    }
                }
                else if(instr->is_fp2si())//类型转化
                {
                    auto a_int=dynamic_cast<ConstantFP*>(instr->get_operand(0));
                    if(cast_constantfp(instr->get_operand(0)))
                    {
                        instr->replace_all_use_with(ConstantInt::get((int)(a_int->get_value()), m_));
                        ins2del.insert(instr);
                    }
                }
                else if(instr->is_cmp())// 整型比较
                {
                    auto l_var = instr->get_operand(0);
                    auto r_var = instr->get_operand(1);
                    if(cast_constantint(l_var) &&  cast_constantint(r_var))
                    {
                        instr->replace_all_use_with(conpute_i_compare(dynamic_cast<CmpInst*>(instr)->get_cmp_op(),  l_var, r_var, m_));
                        ins2del.insert(instr);
                    }
                }
                else if(instr->is_fcmp())// 浮点比较
                {
                    auto l_var = instr->get_operand(0);
                    auto r_var = instr->get_operand(1);
                    if(cast_constantfp(l_var) &&  cast_constantfp(r_var))
                    {
                        instr->replace_all_use_with(compute_f_compare(dynamic_cast<FCmpInst*>(instr)->get_cmp_op(),  l_var, r_var, m_));
                        ins2del.insert(instr);
                    }
                }
                else if(dynamic_cast<BinaryInst*>(instr) != nullptr)//运算
                {
                    BinaryInst* inst = dynamic_cast<BinaryInst*>(instr);
                    auto op = instr->get_instr_type();
                    auto l_var = inst->get_operand(0);
                    auto r_var = inst->get_operand(1);
                    if(cast_constantint(l_var) && cast_constantint(r_var))  
                        inst->replace_all_use_with(compute_int(op, l_var, r_var, m_));
                    else if(cast_constantfp(l_var) && cast_constantfp(r_var))
                        inst->replace_all_use_with(compute_fp(op, l_var, r_var, m_));
                    if(cast_constantint(l_var) && cast_constantint(r_var)||cast_constantfp(l_var) && cast_constantfp(r_var))
                        ins2del.insert(inst);
                }
                
                else if(instr->is_br())// 跳转指令
                {
                    auto brInstr =  dynamic_cast<BranchInst*>(instr);
                    if(!brInstr->is_cond_br())//无条件跳转
                    {
                        auto nextbb = dynamic_cast<BasicBlock*>(brInstr->get_operand(0));
                        if(visitedBB.find(nextbb)==visitedBB.end())
                        {
                              visitedBB.insert(nextbb);
                              blocks.push(nextbb);
                        }
                    }
                    else//条件跳转
                    {
                        auto cond = brInstr->get_operand(0);
                        auto trueBB = dynamic_cast<BasicBlock*>(brInstr->get_operand(1));
                        auto falseBB = dynamic_cast<BasicBlock*>(brInstr->get_operand(2));
                       
                        if(!cast_constantint(cond)&&!cast_constantfp(cond))// 不是常量
                        {
                            if(visitedBB.find(trueBB)==visitedBB.end())//将两个分支都加入工作区
                            {
                                visitedBB.insert(trueBB);
                                blocks.push(trueBB);
                            }
                            if(visitedBB.find(falseBB)==visitedBB.end())
                            {
                                visitedBB.insert(falseBB);
                                blocks.push(falseBB);
                            }
                        }

                        else//是常量
                        {
                            bool flag1,flag2;
                            flag1  = dynamic_cast<ConstantInt*>(cond)->get_value() != 0;
                            flag2  = dynamic_cast<ConstantFP*>(cond)->get_value() != 0;
                            if(flag1||flag2)// 恒成立
                            {
                                brInstr->replace_all_use_with(BranchInst::create_br(trueBB,  bb));
                                if(visitedBB.find(trueBB)==visitedBB.end())
                                {
                                    visitedBB.insert(trueBB);
                                    blocks.push(trueBB);
                                }
                            }
                            else if(!flag1 && !flag2)// 恒不成立
                            {
                                brInstr->replace_all_use_with(BranchInst::create_br(falseBB,  bb));
                                if(visitedBB.find(falseBB)==visitedBB.end())
                                {
                                    visitedBB.insert(falseBB);
                                    blocks.push(falseBB);
                                }
                            }
                            ins2del.insert(instr);
                        }
                    }
                }
            }
                for(auto instr: ins2del)
                bb->delete_instr(instr);//删除无用代码
                }
        }
}
void ConstPropagation::run()
{
    auto func_list = m_->get_functions();
    for (auto func : func_list)
    {
        if(func->get_num_basic_blocks() == 0) continue;
        blocks.push(func->get_entry_block());//加入工作区
        visitedBB.insert(func->get_entry_block());//已经访问
        const_replace();
    }
}