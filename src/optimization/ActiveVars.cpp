#include "ActiveVars.hpp"
#include "set"
#include <map>
#include"vector"
#include"iostream"
#include <unordered_map>
bool judge_set(std::set<Value*>set1,std::set<Value*>set2)
{
    if(set1.size() == set2.size())
    {
    	std::set<Value *>::iterator it1, it2;
    	it1 = set1.begin(); 
    	it2 = set2.begin();
        for ( ; it1 != set1.end(); it1++,it2++)
        {
            if (*it1 == *it2)
            {
                ;
            }
            else
            {
            	return false;
			}
        }
    return true;
	}
	else
	{
		return false;
	}
    
}
void ActiveVars::run()
{
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    output_active_vars << "[";
    for (auto &func : this->m_->get_functions()) {
        if (func->get_basic_blocks().empty()) {
            continue;
        }
        else
        {
            func_ = func;
            live_in.clear();
            live_out.clear();
           
            
            std::set<Value *> used;
            std::set<Value *> defed;
            func_->set_instr_name();
            for (auto bb : func_->get_basic_blocks())
            {
                for (auto inst: bb->get_instructions())
                {
                    if( inst->is_fcmp()|| inst->isBinary() || inst->is_cmp() )
                    {
                        if (defed.find(inst->get_operand(0)) == defed.end()) //first value is a var
                        {
                        	if(dynamic_cast<ConstantFP *>(inst->get_operand(0)) == nullptr)
                        	{
                        		if(dynamic_cast<ConstantInt *>(inst->get_operand(0)) == nullptr)
                        		{
                        			used.insert(inst->get_operand(0));
								}
							}  
                        }
                        if (defed.find(inst->get_operand(1)) == defed.end()) //second value is a var
                        {
                        	if(dynamic_cast<ConstantFP *>(inst->get_operand(1)) == nullptr)
                        	{
                        		if(dynamic_cast<ConstantInt *>(inst->get_operand(1)) == nullptr)
                        		{
                        			used.insert(inst->get_operand(1));
								}
							}
                            
                        }
                        if (used.find(inst) != used.end())
                        {
                            ;
                        }
                        else
                        {
                        	defed.insert(inst);
						}
                    }
                    else if (inst->is_alloca())
                    {
                    	if(used.find(inst) != used.end())
                    	{
                    		;
						}
						else
						{
							defed.insert(inst);
						}
                       
                    }
                    else if (inst->is_gep())
                    {
                    	int i=0;
                        while(i < inst->get_num_operand())
                        {
                            if (defed.find(inst->get_operand(i)) == defed.end() ) //second value is a var
                            {
                            	if(dynamic_cast<ConstantFP *>(inst->get_operand(i)) == nullptr)
                            	{
                            		if(dynamic_cast<ConstantInt *>(inst->get_operand(i)) == nullptr)
                            		{
                            			used.insert(inst->get_operand(i));
									}
								}
                                
                            }
                            if (used.find(inst) != used.end()) 
                            {
                                ;
                            }
                            else
                            {
                            	defed.insert(inst);
							}
							i++;
                        }
                    }
                    
                    else if (inst->is_load())
                    {
						if (used.find(inst) != used.end())
                        {
                            ;
                        }
                        else
                        {
                        	defed.insert(inst);
						}
						auto l_val = inst->get_operand(0);
						if (defed.find(l_val) != defed.end())
                        {
                            ;
                        }
                        else
                        {
                        	used.insert(l_val);
						}
                    }
                    else if(inst->is_store())
                    {
                        auto l_val = static_cast<StoreInst *>(inst)->get_lval();
                        auto r_val = static_cast<StoreInst *>(inst)->get_rval();
                        if(defed.find(l_val) != defed.end())
                        {
                        	;
						}
						
                        else
                        {
                        	if(dynamic_cast<ConstantInt *>(l_val) == nullptr)
                        	{
                        		used.insert(l_val);
							}
                            used.insert(l_val);
                        }
                        if ((defed.find(r_val) == defed.end()) )
                        {
                        	if(dynamic_cast<ConstantInt *>(r_val) == nullptr)
                        	{
                        		if(dynamic_cast<ConstantFP *>(r_val) != nullptr)
                        		{
                        			;
								}
                        		else
                        		{
                        			used.insert(r_val);
								}
							}
                            
                        }
                        
                    }
                    else if (inst->is_call())
                    {
                    	int i=1;
                        while (i < inst->get_num_operand())
                        {
                        	if(defed.find(inst->get_operand(i)) != defed.end())
                        	{
                        		;
							}
                            else
                            {
                            	if(dynamic_cast<ConstantInt *>(inst->get_operand(i)) == nullptr)
                            	{
                            		if(dynamic_cast<ConstantFP *>(inst->get_operand(i)) != nullptr)
                            		{
                            			;
									 } 
                            		else
                            		{
                            			 used.insert(inst->get_operand(i));
									}
								}
                            }
                            i++;
                        }
                        if(used.find(inst) != used.end())
                        {
                        	;
						}
                        else
                        {
                        	if(!inst->is_void())
                            defed.insert(inst);
                        }
                    }
                    
                    
                    else if (inst->is_ret())
                    {
                        if ((static_cast<ReturnInst *>(inst)->is_void_ret() == false) )
                        {
                        	if(defed.find(inst->get_operand(0)) == defed.end())
                        	{
                        		if(dynamic_cast<ConstantInt *>(inst->get_operand(0)) == nullptr)
                        		{
                        			if(dynamic_cast<ConstantFP *>(inst->get_operand(0)) != nullptr)
                        			{
                        				;
									}
                        			else
                        			{
                        				used.insert(inst->get_operand(0));
									}
								}
							}
                            
                        }
                    }
                    else if (inst->is_phi())
                    {
                    	int i=0;
                        while (i < inst->get_num_operand()/2)
                        {
                            if(defed.find(inst->get_operand(2*i)) != defed.end())
                            {
                            	;
							}
                            else
                            {
                                if (dynamic_cast<ConstantFP *>(inst->get_operand(2*i)) != nullptr)
                                {
                                	;
								}
                                else
                                {
                                	if(dynamic_cast<ConstantInt *>(inst->get_operand(2 * i)) == nullptr)
                                    {
                                    	big_phi[bb].insert({inst->get_operand(2 * i), dynamic_cast<BasicBlock *>(inst->get_operand(2 * i + 1))});
                                        used.insert(inst->get_operand(2 * i));
									}
                                    
                                }
                            }
                            i++;
                        }
                        if (used.find(inst) == used.end())
                        {
                            defed.insert(inst);
                        }
                    }
                    else if (inst->is_si2fp()|| inst->is_zext() || inst->is_fp2si())
                    {
                    	if (used.find(inst) == used.end())
                        {
                            defed.insert(inst);
                        }
                        if (defed.find(inst->get_operand(0)) == defed.end())
                        {
                            used.insert(inst->get_operand(0));
                        }
                    }
                    
                    
                    else if (inst->is_br())
                    {
                        if (defed.find(inst->get_operand(0)) == defed.end())
                        {
                        	if(dynamic_cast<ConstantFP *>(inst->get_operand(0)) == nullptr)
                        	{
                        		if(dynamic_cast<ConstantInt *>(inst->get_operand(0)) == nullptr)
                        		{
                        			if (static_cast<BranchInst*>(inst)->is_cond_br())
                                    {
                                        used.insert(inst->get_operand(0));
                                    }
                                    else
                                    {
                                    	;
									}
								}
							}
                            
                        }
                    }
                }
                map_use.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, used));
                map_def.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, defed));
                used.clear();
                defed.clear();
                
            }
            std::set<Value *> temp_outb;
            std::set<Value *> union_ins;
            std::set<Value *> in_reserve;
            bool is_change = true;
            for(auto bb : func->get_basic_blocks())
            {
                std::set<Value *> initial_set;
                live_in.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, initial_set));
            }
            while (is_change)
            {
                is_change = false;
                for (auto bb : func->get_basic_blocks())
                {
                    for (auto succ_bb : bb->get_succ_basic_blocks())
                    {
                    	std::set<Value *> old_ins;
                        auto INS = live_in.find(succ_bb);
                        while (judge_set(INS->second, old_ins) != true)
                        {
                            old_ins = INS->second;
                            for (auto item : INS->second)
                            {
                            	if(big_phi.find(succ_bb) == big_phi.end())
                            	{
                            		union_ins.insert(item);
								}
								else
								{
									if(big_phi.find(succ_bb)->second.find(item) == big_phi.find(succ_bb)->second.end())
									{
										 union_ins.insert(item);
									}
									else
									{
										if (big_phi.find(succ_bb)->second.find(item)->second->get_name() == bb->get_name())
                                        {
                                            union_ins.insert(item);

                                        }
									}
								}
                               
                            }
                        }
                    }
                    if(live_out.find(bb) != live_out.end())
                    {
                    	live_out.find(bb)->second = union_ins;
					}
					else
					{
						live_out.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, union_ins));
					}
                    temp_outb = live_out.find(bb)->second;
                    for(auto item : map_def.find(bb)->second)
					{
                        if (temp_outb.find(item) == temp_outb.end())
                        {
                            ;
                        }
                        else
                        {
                        	temp_outb.erase(item);
						}
                    }
                    in_reserve = live_in.find(bb)->second;
                    for(auto item:temp_outb)
                    {
                        map_use.find(bb)->second.insert(item);
                    }
                    live_in.find(bb)->second = map_use.find(bb)->second;
                    if (judge_set(in_reserve,live_in.find(bb)->second) != true)
                    {
                        is_change = true;
                    }

                    union_ins.clear();
                    temp_outb.clear();
                    in_reserve.clear();
                }
            }
            output_active_vars << print();
            output_active_vars << ",";
        }
        map_use.clear();
        map_def.clear();
        big_phi.clear();
    }
    output_active_vars << "]";
    output_active_vars.close();
    return ;
}

std::string ActiveVars::print()
{
    std::string active_vars;
    active_vars +=  "{\n";
    active_vars +=  "\"function\": \"";
    active_vars +=  func_->get_name();
    active_vars +=  "\",\n";

    active_vars +=  "\"live_in\": {\n";
    for (auto &p : live_in) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]" ;
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars +=  "    },\n";

    active_vars +=  "\"live_out\": {\n";
    for (auto &p : live_out) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}
