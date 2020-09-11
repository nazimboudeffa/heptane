#include "Logger.h"
#include "StackAnalysis.h"
#include "Generic/CallGraph.h"
#include "Generic/ContextHelper.h"
#include "arch.h"

/**
 * This is used to check if the stack pointer is modified in the middle of a function
 *
 * @return true if sp is modified, false otherwise.
 */
bool StackAnalysis::VerifModifStack (Cfg * cfg)
{
  Node *firstNode = cfg->GetStartNode ();
  const vector < Node * >&endNodes = cfg->GetEndNodes ();
  const vector < Node * >&listNodes = cfg->GetAllNodes ();

  for (size_t n = 0; n < listNodes.size (); n++)
    if (listNodes[n] != firstNode)	// not first node
      if (std::find (endNodes.begin (), endNodes.end (), listNodes[n]) == endNodes.end ())	//not in endNodes
	{
	  const vector < Instruction * >&listInstr = listNodes[n]->GetAsm ();
	  // For each instruction in node
	  for (size_t i = 0; i < listInstr.size (); i++)
	    {
	      string s_instr = listInstr[i]->GetCode ();
	      vector < string > output_registers = Arch::getResourceOutputs (s_instr);
	      for (size_t j = 0; j < output_registers.size (); j++)
		{
		  if (output_registers[j] == "sp") return true;
		}
	    }
	}
  return false;
}

/**
 * This is used to get the maximum offset relative to sp for instruction like : mnemonic $1,val($sp) of a cfg
 * @return the size of the stack frame including the parameter present in the caller function if used.
 */
int StackAnalysis::getStackMaxOffset (Cfg * cfg, int stack_size)
{
  int result = stack_size;

  const vector < Node * >&nodes = cfg->GetAllNodes ();
  for (size_t n = 0; n < nodes.size (); n++)
    {
      const vector < Instruction * >&instructions = nodes[n]->GetAsm ();

      // For each instruction in the current node
      for (size_t i = 0; i < instructions.size (); i++)
	{
	  // if (! instructions[i]->HasAttribute(MetaInstructionAttributeName)) // Added lbesnard Sept 2017.
	    {
	      string s_instr = instructions[i]->GetCode ();
	      if (Arch::isLoad (s_instr) || Arch::isStore (s_instr))
		result = getStackMaxOffset(s_instr, result); // ARCH Dependant.
	    }
	}
    }
  return result;
}

void StackAnalysis::stackAnalysis(Cfg * current_cfg)
{
  assert (! current_cfg->HasAttribute (StackInfoAttributeName));
  StackInfoAttribute attribute;
  
  //set the stackFrameSizeWithoutCaller
  int stack_size = getStackSize (current_cfg);
  attribute.setFrameSizeWithoutCaller (stack_size);
  
  int maxOffset = getStackMaxOffset (current_cfg, stack_size);
  attribute.setFrameSizeWithCaller (maxOffset);
  
  //attach the attribute to the current cfg
  current_cfg->SetAttribute (StackInfoAttributeName, attribute);
}


bool StackAnalysis::stackAnalysisInit(CallGraph &callgraph)
{
  vector < Cfg * >listCfg = p->GetAllCfgs ();
  for (size_t cfg = 0; cfg < listCfg.size (); cfg++)
    {
      Cfg * current_cfg = listCfg[cfg];
      if ( ! callgraph.isDeadCode (current_cfg)) // Ignore dead cfgs
	if (VerifModifStack (current_cfg))
	  Logger::addError ("modification of the stack frame in the middle of function:" + current_cfg->getStringName());
    }

  if (Logger::getErrorState ()) { Logger::print (); return false; }
  return true;
}


/**
   Stack analysis starting point
*/
bool
StackAnalysis::stackAnalysis()
{
  CallGraph callgraph (p);
  if ( !stackAnalysisInit(callgraph)) return false;

  //the analysis
  vector < Cfg * >listCfg = p->GetAllCfgs ();
  //compute the stack size information
  for (size_t cfg = 0; cfg < listCfg.size (); cfg++)
    {
      Cfg * current_cfg = listCfg[cfg];
      if (! callgraph.isDeadCode (current_cfg))
	stackAnalysis(current_cfg);
    }

  // compute the [sp by contexts] values
  map < Context *, long >sp_values;
  sp_values[NULL] = spinit;

  const ContextTree & contexts = (ContextTree &) p->GetAttribute (ContextTreeAttributeName);

  // For each context in the tree
  for (size_t c = 0; c < contexts.getContextsCount (); ++c)
    {
      Context *context = contexts.getContext (c);
      Cfg *currentCfg = context->getCurrentFunction ();

      //get the StackInfoAttributeName of current cfg
      assert (currentCfg->HasAttribute (StackInfoAttributeName));
      StackInfoAttribute attribute = (StackInfoAttribute &) currentCfg->GetAttribute (StackInfoAttributeName);

      //get the stack size without caller of current cfg
      long stack_size = attribute.getFrameSizeWithoutCaller ();

      // Find caller sp and current function stack size
      assert (sp_values.find (context->getCallerContext ()) != sp_values.end ());
      long caller_sp = sp_values[context->getCallerContext ()];

      // Compute and save current context sp value
      long sp_value = caller_sp - stack_size;
      sp_values[context] = sp_value;

      //add (sp_value ; ctx_str) to the attribute if not already present
      attribute.addContext (sp_value, context->getStringId ());

      //attach the attribute to the current cfg
      currentCfg->SetAttribute (StackInfoAttributeName, attribute);
    }

  return true;
}


StackAnalysis::StackAnalysis(Program *p, int sp):Analysis(p)
{
  spinit = sp;
}
