
#ifndef ABSTRACTREGMEM_H
#define ABSTRACTREGMEM_H

class AbstractRegMem
{
  RegState* regs;

 public:

  AbstractRegMem (RegState *rm)
    { 
      regs = rm;
    };

  AbstractRegMem ()
    {
      regs = NULL; 
    };
 
  RegState* getRegState()
  {
    return regs;
  }

  void setRegisters(const AbstractRegMem &v)
  {
    regs->setRegisters( v.regs);
  }
 
  void setStack(const AbstractRegMem &v)
  {
    regs->setStack( v.regs);
  }

  bool EqualsRegisters (const AbstractRegMem &c) const
  {
    return regs->EqualsRegisters(c.regs);
  }

  bool EqualsStacks (const AbstractRegMem &c) const
  {
    return regs->EqualsStacks(c.regs);
  }
 
  /** Join function */
  void JoinRegisters (const AbstractRegMem &c)
  {
    regs->JoinRegisters(c.regs);    
  }

  /** Join function */
  void JoinStacks (const AbstractRegMem &c)
  {
    regs->JoinStacks(c.regs);    
  }

  void reset()
  {
    regs->reset_sp();
  }

  void print()
  {
    regs->printStates();
    regs->printStack();
  }
};


class AbstractRegMemAttribute:public NonSerialisableAttribute
{
 public:
  AbstractRegMem regmem;

  /** Constructor */
  AbstractRegMemAttribute (const AbstractRegMem &rm)
    {
      regmem = rm;
    };
  
  AbstractRegMem getAbstractRegMem()
  {
    return regmem;
  }

  void setAbstractRegMemRegisters(AbstractRegMem v)
  {
    regmem.setRegisters(v);
  }

  void setAbstractRegMemStack(AbstractRegMem v)
  {
    regmem.setStack(v);
  }


  /** Cloning function */
  AbstractRegMemAttribute *clone ()
  {
    return new AbstractRegMemAttribute(regmem);
  };

  /** Printing function */
  void Print (std::ostream & os)
  {
    os << "(type NonSerialisable AbstractRegMemAttribute, name " << name << ")";
  };
};

#endif
