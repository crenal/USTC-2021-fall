#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"
using namespace std;
void LoopInvHoist::run()
{
    // 先通过LoopSearch获取循环的相关信息
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();

    // 接下来由你来补充啦！
    m_->set_print_name(); //记得给phi结点命名！！
    auto func_list = m_->get_functions();
    for(auto func : func_list){
        for(auto bb_set: loop_searcher.get_loops_in_func(func)){  //into loop
            auto base = loop_searcher.get_loop_base(bb_set);
            auto prevs = base->get_pre_basic_blocks(); // find previous bb of loopbase
            if (prevs.size() == 0)                     // no pre bbs
            {
                continue;
            }

            std::unordered_set<std::string> def_set; // store defined variables in a loop
            // find defined variables
            cout <<"def:";
            for (auto bb : *bb_set)
            {
                for (auto instr : bb->get_instructions())
                {
                    auto def_var = instr->get_name();
                    if (def_var!= ""){
                        cout << def_var << " ";
                        def_set.insert(def_var);
                    }
                }
            }cout << endl;

            std::set<Instruction *> hoist_set;         //use set to ensure the order的

            bool hoisted; // if hoisted then check again
            // check whether needed to hoist
            do
            {
                hoisted = false;
                for (auto bb : *bb_set)
                {
                    for (auto instr : bb->get_instructions())
                    {
                        if (hoist_set.find(instr) != hoist_set.end() || instr->is_phi() || instr->is_store() || instr->is_alloca() || instr->is_ret() || instr->is_load() || instr->is_br())
                        {
                            continue;
                        }

                        bool defined = false;
                        for (auto operand : instr->get_operands())
                        {
                            auto opname = operand->get_name();
                            if (def_set.find(opname) != def_set.end())
                            {
                                cout << opname << " ";
                                defined = true;
                                break;
                            }
                        }
                        if(defined)
                        {
                            continue;
                        }
                        else
                        {
                            hoisted = true;
                            hoist_set.insert(instr);
                            def_set.erase(instr->get_name());
                        }
                        cout <<"def:";for(auto def: def_set)cout<<def<< " ";cout<<endl;
                    }
                }cout << "find, don't hoist"<<endl;
            } while (hoisted);
            cout << "to hoist:";
            for(auto hoist:hoist_set)
                cout << hoist->get_name() << " ";
            cout << endl;
            auto prev = prevs.front();
            if (prevs.size() == 2)
            {
                auto prev2 = prevs.back();
                if (bb_set->find(prev) != bb_set->end())
                {
                    prev = prev2;
                }
            }
            bool insert = false; // check the last instruction of
            auto last_ins = prev->get_instructions().back();
            if (last_ins->is_br() || last_ins->is_ret())
            {
                insert = true;
            }
            if (insert)
                    prev->delete_instr(last_ins);
            for (auto instr = hoist_set.begin(); instr != hoist_set.end(); instr++)
            {
                auto ins = *instr;
                ins->get_parent()->delete_instr(ins);
                ins->set_parent(prev);
                
                prev->add_instruction(ins);
                
            }
            if (insert)
                    prev->add_instruction(last_ins);
            }
    }
}
