#pragma once
#include <string>
#include <vector>

/**
  * String interpreter used to parse and execute formatted 
  * telemetry functions and commands.
  */
class Interpreter
{
public:

  enum FunctionTypeEnum
  {
    VALUE,
    MAP_TO_INDEX,
    READ_8,
    READ_8I,
    READ_16,
    READ_16I,
    READ_FLOAT,
    READ_32,
    READ_32I,
    READ_ARRAY,
    WRITE_8,
    WRITE_8I,
    WRITE_16,
    WRITE_16I,
    WRITE_FLOAT,
    WRITE_32,
    WRITE_32I,
    WRITE_ARRAY,
    SET_FLAG,
    HELP,
    MAX_FUNCTION_TYPE
  };

  enum ModifierTypeEnum
  {
    FMULT,
    FDIV,
    FADD,
    FSUB,
    CMP_EQ,
    CMP_NE,
    CMP_G,
    CMP_GE,
    CMP_L,
    CMP_LE,
    MAX_MODIFIER_TYPE
  };

  struct Function
  {
    FunctionTypeEnum         Type;
    std::vector<std::string> Parameters;
    ModifierTypeEnum         Modifier;
    std::string              ModifierValue;
  };

  struct ReturnType
  {
    bool              bRet;
    uint8_t           u8Ret;
    int8_t            i8Ret;
    uint16_t          u16Ret;
    int16_t           i16Ret;
    uint32_t          u32Ret;
    int32_t           i32Ret;
    uintptr_t         u64Ret;
    float             fRet;
    std::vector<char> arrRet;
  };

  static Function Translate(std::string str);
  static uintptr_t ToInteger(std::string strValue);
  static std::string GetFunctionString(FunctionTypeEnum type);
  static std::string GetModifierString(ModifierTypeEnum type);
  static ReturnType InvokeFunction(Function& function, void* param);


  static bool DeveloperMode;
  static std::string HelpPrompt;
  static std::string HelpPromptDev;
};

