#include "Interpreter.h"
#include "MemUtils.h"
#include "TelemetryManager.h"

bool Interpreter::DeveloperMode = false;
std::string Interpreter::HelpPrompt = 
  "read8u (_in addr, _in_opt ptr_offset, ...)\n"
  "read8i (_in addr, _in_opt ptr_offset, ...)\n"
  "read16u(_in addr, _in_opt ptr_offset, ...)\n"
  "read16i(_in addr, _in_opt ptr_offset, ...)\n"
  "read32u(_in addr, _in_opt ptr_offset, ...)\n"
  "read32i(_in addr, _in_opt ptr_offset, ...)\n"
  "readF  (_in addr, _in_opt ptr_offset1, ...)\n"
  "read[] (_in len, _in addr, _in_opt ptr_offset, ...)";

std::string Interpreter::HelpPromptDev = Interpreter::HelpPrompt + "\n" +
  "write8u (_in value, _in addr, _in_opt ptr_offset, ...)\n"
  "write8i (_in value, _in addr, _in_opt ptr_offset, ...)\n"
  "write16u(_in value, _in addr, _in_opt ptr_offset, ...)\n"
  "write16i(_in value, _in addr, _in_opt ptr_offset, ...)\n"
  "write32u(_in value, _in addr, _in_opt ptr_offset, ...)\n"
  "write32i(_in value, _in addr, _in_opt ptr_offset, ...)\n"
  "writeF  (_in value, _in addr, _in_opt ptr_offset, ...)\n"
  "write[] (_in value, _in addr, _in_opt ptr_offset, ...)";

/**
 * Function: Translate
 * Notes: See header file
 */
Interpreter::Function Interpreter::Translate(std::string str)
{
  Interpreter::Function function;

  function.Type = MAX_FUNCTION_TYPE;
  function.Modifier = MAX_MODIFIER_TYPE;
  if (0 != str.length())
  {
    // First remove all spaces
    size_t spacePos = str.find(" ");
    while (std::string::npos != spacePos)
    {
      str = str.substr(0, spacePos) + str.substr(spacePos + 1);
      spacePos = str.find(" ");
    }

    size_t paramStartPos = str.find_first_of('(');
    if (std::string::npos == paramStartPos)
    {
      function.Type = VALUE;
      function.Parameters.push_back(str);
      return function;
    }

    size_t paramEndPos = str.find_first_of(')');
    if (std::string::npos == paramEndPos)
    {
      function.Type = VALUE;
      function.Parameters.push_back(str);
      return function;
    }

    // Check for a modifier which is only valid for functions
    size_t modifierPos = str.find("*", paramEndPos);
    if (std::string::npos != modifierPos)
    {
      function.Modifier = FMULT;
    }
    else if (std::string::npos != (modifierPos = str.find("/", paramEndPos)))
    {
      function.Modifier = FDIV;
    }
    else if (std::string::npos != (modifierPos = str.find("+", paramEndPos)))
    {
      function.Modifier = FADD;
    }
    else if (std::string::npos != (modifierPos = str.find("-", paramEndPos)))
    {
      function.Modifier = FSUB;
    }
    else if (std::string::npos != (modifierPos = str.find("==", paramEndPos)))
    {
      function.Modifier = CMP_EQ;
      ++modifierPos;
    }
    else if (std::string::npos != (modifierPos = str.find("!=", paramEndPos)))
    {
      function.Modifier = CMP_NE;
      ++modifierPos;
    }
    else if (std::string::npos != (modifierPos = str.find(">=", paramEndPos)))
    {
      function.Modifier = CMP_GE;
      ++modifierPos;
    }
    else if (std::string::npos != (modifierPos = str.find(">", paramEndPos)))
    {
      function.Modifier = CMP_G;
    }
    else if (std::string::npos != (modifierPos = str.find("<=", paramEndPos)))
    {
      function.Modifier = CMP_LE;
      ++modifierPos;
    }
    else if (std::string::npos != (modifierPos = str.find("<", paramEndPos)))
    {
      function.Modifier = CMP_L;
    }

    if (MAX_MODIFIER_TYPE != function.Modifier)
    {
      function.ModifierValue = str.substr(modifierPos + 1);
    }

    std::string functionStr = str.substr(0, paramStartPos);
    if ("map" == functionStr)
    {
      function.Type = MAP_TO_INDEX;
    }
    else if ("read8u" == functionStr)
    {
      function.Type = READ_8;
    }
    else if ("read8i" == functionStr)
    {
      function.Type = READ_8I;
    }
    else if ("read16u" == functionStr)
    {
      function.Type = READ_16;
    }
    else if ("read16i" == functionStr)
    {
      function.Type = READ_16I;
    }
    else if ("readF" == functionStr)
    {
      function.Type = READ_FLOAT;
    }
    else if ("read32u" == functionStr)
    {
      function.Type = READ_32;
    }
    else if ("read32i" == functionStr)
    {
      function.Type = READ_32I;
    }
    else if ("read[]" == functionStr)
    {
      function.Type = READ_ARRAY;
    }
    else if ("write8u" == functionStr)
    {
      function.Type = WRITE_8;
    }
    else if ("write8i" == functionStr)
    {
      function.Type = WRITE_8I;
    }
    else if ("write16u" == functionStr)
    {
      function.Type = WRITE_16;
    }
    else if ("write16i" == functionStr)
    {
      function.Type = WRITE_16I;
    }
    else if ("writeF" == functionStr)
    {
      function.Type = WRITE_FLOAT;
    }
    else if ("write32u" == functionStr)
    {
      function.Type = WRITE_32;
    }
    else if ("write32i" == functionStr)
    {
      function.Type = WRITE_32I;
    }
    else if ("write[]" == functionStr)
    {
      function.Type = WRITE_ARRAY;
    }
    else if ("setflag" == functionStr)
    {
      function.Type = SET_FLAG;
    }
    else if ("help" == functionStr)
    {
      function.Type = HELP;
    }
    else
    {
      function.Type = VALUE;
    }

    std::string parameters = str.substr(paramStartPos + 1, paramEndPos - paramStartPos - 1);
    std::vector<char> param_buf(parameters.size() + 1, 0);
    memcpy(param_buf.data(), parameters.c_str(), param_buf.size());

    char* token = strtok(param_buf.data(), ",");
    while (nullptr != token)
    {
      function.Parameters.push_back(token);
      token = strtok(nullptr, ",");
    }
  }

  return function;
}

/**
 * Function: ToInteger
 * Notes: See header file
 */
uintptr_t Interpreter::ToInteger(std::string strValue)
{
  if (std::string::npos != strValue.find("0x"))
  {
    return strtoull(strValue.c_str(), 0, 16);
  }

  return strtoull(strValue.c_str(), 0, 10);
}

/**
 * Function: GetFunctionString
 * Notes: See header file
 */
std::string Interpreter::GetFunctionString(FunctionTypeEnum type)
{
  std::string function = "";
  switch (type)
  {
  case FunctionTypeEnum::VALUE:
    function = "value";
    break;
  case FunctionTypeEnum::MAP_TO_INDEX:
    function = "map";
    break;
  case FunctionTypeEnum::READ_8:
    function = "read8u";
    break;
  case FunctionTypeEnum::READ_8I:
    function = "read8i";
    break;
  case FunctionTypeEnum::READ_16:
    function = "read16u";
    break;
  case FunctionTypeEnum::READ_16I:
    function = "read16i";
    break;
  case FunctionTypeEnum::READ_FLOAT:
    function = "readF";
    break;
  case FunctionTypeEnum::READ_32:
    function = "read32u";
    break;
  case FunctionTypeEnum::READ_32I:
    function = "read32i";
    break;
  case FunctionTypeEnum::READ_ARRAY:
    function = "read[]";
    break;
  case FunctionTypeEnum::WRITE_8:
    function = "write8u";
    break;
  case FunctionTypeEnum::WRITE_8I:
    function = "write8i";
    break;
  case FunctionTypeEnum::WRITE_16:
    function = "write16u";
    break;
  case FunctionTypeEnum::WRITE_16I:
    function = "write16i";
    break;
  case FunctionTypeEnum::WRITE_FLOAT:
    function = "writeF";
    break;
  case FunctionTypeEnum::WRITE_32:
    function = "write32u";
    break;
  case FunctionTypeEnum::WRITE_32I:
    function = "write32i";
    break;
  case FunctionTypeEnum::WRITE_ARRAY:
    function = "write[]";
    break;
  }

  return function;
}

std::string Interpreter::GetModifierString(ModifierTypeEnum type)
{
  std::string modifier = "";
  switch (type)
  {
     case FMULT:
        modifier = "*";
        break;
     case FDIV:
        modifier = "/";
        break;
     case FADD:
        modifier = "+";
        break;
     case FSUB:
        modifier = "-";
        break;
     case CMP_EQ:
        modifier = "==";
        break;
     case CMP_NE:
        modifier = "!=";
        break;
     case CMP_G:
        modifier = ">";
        break;
     case CMP_GE:
        modifier = ">=";
        break;
     case CMP_L:
        modifier = "<";
        break;
     case CMP_LE:
        modifier = "<=";
        break;
  }

  return modifier;
}

/**
 * Function: InvokeFunction
 * Notes: See header file
 */
Interpreter::ReturnType Interpreter::InvokeFunction(Function& function, void* param)
{
  MemUtils* memUtil = reinterpret_cast<MemUtils*>(param);

  Interpreter::ReturnType funcRet;
  if (0 != function.Parameters.size())
  {
    float modValue = (MAX_MODIFIER_TYPE != function.Modifier) ? strtof(function.ModifierValue.c_str(), nullptr) : 0;

    switch (function.Type)
    {
      // map(index)
      case Interpreter::MAP_TO_INDEX:
      {
        funcRet.u32Ret = Interpreter::ToInteger(function.Parameters[0]);
        funcRet.i32Ret = Interpreter::ToInteger(function.Parameters[0]);    
      }
      break;
      // read8(address,ptr_offset1,ptr_offset2,...)
      case Interpreter::READ_8:
      case Interpreter::READ_8I:
      {
        uintptr_t address = memUtil->MapAddress(function.Parameters[0]);
        if (1 < function.Parameters.size())
        {
          std::vector<uintptr_t> addresses(function.Parameters.size(), 0);
          addresses[0] = address;
          for (size_t i = 1; i < function.Parameters.size(); ++i)
          {
            addresses[i] = Interpreter::ToInteger(function.Parameters[i]);
          }

          address = memUtil->GetPointerAddress(memUtil->GetActiveProcess(), addresses);
        }

        funcRet.u8Ret = memUtil->Read8(memUtil->GetActiveProcess(), address);
        if (funcRet.u8Ret <= CHAR_MAX)
        {
          funcRet.i8Ret = static_cast<int8_t>(funcRet.u8Ret);
        }
        else
        {
          funcRet.i8Ret = static_cast<int8_t>(funcRet.u8Ret - CHAR_MIN) + CHAR_MIN;
        }

        switch (function.Modifier)
        {
        case FMULT:
          if (READ_8I == function.Type) funcRet.i8Ret = funcRet.i8Ret * modValue;
          else funcRet.u8Ret = funcRet.u8Ret = funcRet.u8Ret * modValue;
          break;
        case FDIV:
          if (READ_8I == function.Type) funcRet.i8Ret = funcRet.i8Ret / modValue;
          else funcRet.u8Ret = funcRet.u8Ret = funcRet.u8Ret / modValue;
          break;
        case FADD:
          if (READ_8I == function.Type) funcRet.i8Ret = funcRet.i8Ret + modValue;
          else funcRet.u8Ret = funcRet.u8Ret = funcRet.u8Ret + modValue;
          break;
        case FSUB:
          if (READ_8I == function.Type) funcRet.i8Ret = funcRet.i8Ret - modValue;
          else funcRet.u8Ret = funcRet.u8Ret = funcRet.u8Ret - modValue;
          break;
        case CMP_EQ:
          if (READ_8I == function.Type) funcRet.i8Ret = funcRet.i8Ret == modValue ? 1 : 0;
          else funcRet.u8Ret = funcRet.u8Ret = funcRet.u8Ret == modValue ? 1 : 0;
          break;
        case CMP_NE:
          if (READ_8I == function.Type) funcRet.i8Ret = funcRet.i8Ret != modValue ? 1 : 0;
          else funcRet.u8Ret = funcRet.u8Ret = funcRet.u8Ret != modValue ? 1 : 0;
          break;
        case CMP_G:
          if (READ_8I == function.Type) funcRet.i8Ret = funcRet.i8Ret > modValue ? 1 : 0;
          else funcRet.u8Ret = funcRet.u8Ret = funcRet.u8Ret > modValue ? 1 : 0;
          break;
        case CMP_GE:
          if (READ_8I == function.Type) funcRet.i8Ret = funcRet.i8Ret >= modValue ? 1 : 0;
          else funcRet.u8Ret = funcRet.u8Ret = funcRet.u8Ret >= modValue ? 1 : 0;
          break;
        case CMP_L:
          if (READ_8I == function.Type) funcRet.i8Ret = funcRet.i8Ret < modValue ? 1 : 0;
          else funcRet.u8Ret = funcRet.u8Ret = funcRet.u8Ret < modValue ? 1 : 0;
          break;
        case CMP_LE:
          if (READ_8I == function.Type) funcRet.i8Ret = funcRet.i8Ret <= modValue ? 1 : 0;
          else funcRet.u8Ret = funcRet.u8Ret = funcRet.u8Ret <= modValue ? 1 : 0;
          break;
        }
      }
      break;
      // read16(address,ptr_offset1,ptr_offset2,...)
      case Interpreter::READ_16:
      case Interpreter::READ_16I:
      {
        uintptr_t address = memUtil->MapAddress(function.Parameters[0]);
        if (1 < function.Parameters.size())
        {
          std::vector<uintptr_t> addresses(function.Parameters.size(), 0);
          addresses[0] = address;
          for (size_t i = 1; i < function.Parameters.size(); ++i)
          {
            addresses[i] = Interpreter::ToInteger(function.Parameters[i]);
          }

          address = memUtil->GetPointerAddress(memUtil->GetActiveProcess(), addresses);
        }

        funcRet.u16Ret = memUtil->Read16(memUtil->GetActiveProcess(), address);
        if (funcRet.u16Ret <= SHRT_MAX)
        {
          funcRet.i16Ret = static_cast<int16_t>(funcRet.u16Ret);
        }
        else
        {
          funcRet.i16Ret = static_cast<int16_t>(funcRet.u16Ret - SHRT_MIN) + SHRT_MIN;
        }

        switch (function.Modifier)
        {
        case FMULT:
          if (READ_16I == function.Type) funcRet.i16Ret = funcRet.i16Ret * modValue;
          else funcRet.u16Ret = funcRet.u16Ret = funcRet.u16Ret * modValue;
          break;
        case FDIV:
          if (READ_16I == function.Type) funcRet.i16Ret = funcRet.i16Ret / modValue;
          else funcRet.u16Ret = funcRet.u16Ret = funcRet.u16Ret / modValue;
          break;
        case FADD:
          if (READ_16I == function.Type) funcRet.i16Ret = funcRet.i16Ret + modValue;
          else funcRet.u16Ret = funcRet.u16Ret = funcRet.u16Ret + modValue;
          break;
        case FSUB:
          if (READ_16I == function.Type) funcRet.i16Ret = funcRet.i16Ret - modValue;
          else funcRet.u16Ret = funcRet.u16Ret = funcRet.u16Ret - modValue;
          break;
        case CMP_EQ:
          if (READ_16I == function.Type) funcRet.i16Ret = funcRet.i16Ret == modValue ? 1 : 0;
          else funcRet.u16Ret = funcRet.u16Ret = funcRet.u16Ret == modValue ? 1 : 0;
          break;
        case CMP_NE:
          if (READ_16I == function.Type) funcRet.i16Ret = funcRet.i16Ret != modValue ? 1 : 0;
          else funcRet.u16Ret = funcRet.u16Ret = funcRet.u16Ret != modValue ? 1 : 0;
          break;
        case CMP_G:
          if (READ_16I == function.Type) funcRet.i16Ret = funcRet.i16Ret > modValue ? 1 : 0;
          else funcRet.u16Ret = funcRet.u16Ret = funcRet.u16Ret > modValue ? 1 : 0;
          break;
        case CMP_GE:
          if (READ_16I == function.Type) funcRet.i16Ret = funcRet.i16Ret >= modValue ? 1 : 0;
          else funcRet.u16Ret = funcRet.u16Ret = funcRet.u16Ret >= modValue ? 1 : 0;
          break;
        case CMP_L:
          if (READ_16I == function.Type) funcRet.i16Ret = funcRet.i16Ret < modValue ? 1 : 0;
          else funcRet.u16Ret = funcRet.u16Ret = funcRet.u16Ret < modValue ? 1 : 0;
          break;
        case CMP_LE:
          if (READ_16I == function.Type) funcRet.i16Ret = funcRet.i16Ret <= modValue ? 1 : 0;
          else funcRet.u16Ret = funcRet.u16Ret = funcRet.u16Ret <= modValue ? 1 : 0;
          break;
        }
      }
      break;
      // readF(address,ptr_offset1,ptr_offset2,...)
      case Interpreter::READ_FLOAT:
      {
        uintptr_t address = memUtil->MapAddress(function.Parameters[0]);
        if (1 < function.Parameters.size())
        {
          std::vector<uintptr_t> addresses(function.Parameters.size(), 0);
          addresses[0] = address;
          for (size_t i = 1; i < function.Parameters.size(); ++i)
          {
            addresses[i] = Interpreter::ToInteger(function.Parameters[i]);
          }

          address = memUtil->GetPointerAddress(memUtil->GetActiveProcess(), addresses);
        }

        funcRet.fRet = memUtil->ReadFloat(memUtil->GetActiveProcess(), address);

        switch (function.Modifier)
        {
        case FMULT:
          funcRet.fRet = funcRet.fRet * modValue;
          break;
        case FDIV:
          funcRet.fRet = funcRet.fRet / modValue;
          break;
        case FADD:
          funcRet.fRet = funcRet.fRet + modValue;
          break;
        case FSUB:
          funcRet.fRet = funcRet.fRet - modValue;
          break;
        case CMP_EQ:
          funcRet.fRet = funcRet.fRet == modValue ? 1 : 0;
          break;
        case CMP_NE:
          funcRet.fRet = funcRet.fRet == modValue ? 0 : 1;
          break;
        case CMP_G:
          funcRet.fRet = funcRet.fRet > modValue ? 1 : 0;
          break;
        case CMP_GE:
          funcRet.fRet = funcRet.fRet >= modValue ? 1 : 0;
          break;
        case CMP_L:
          funcRet.fRet = funcRet.fRet < modValue ? 1 : 0;
          break;
        case CMP_LE:
          funcRet.fRet = funcRet.fRet <= modValue ? 1 : 0;
          break;
        }
      }
      break;
      // read32(address,ptr_offset1,ptr_offset2,...)
      case Interpreter::READ_32:
      case Interpreter::READ_32I:
      {
        uintptr_t address = memUtil->MapAddress(function.Parameters[0]);
        if (1 < function.Parameters.size())
        {
          std::vector<uintptr_t> addresses(function.Parameters.size(), 0);
          addresses[0] = address;
          for (size_t i = 1; i < function.Parameters.size(); ++i)
          {
            addresses[i] = Interpreter::ToInteger(function.Parameters[i]);
          }

          address = memUtil->GetPointerAddress(memUtil->GetActiveProcess(), addresses);
        }

        funcRet.u32Ret = memUtil->Read32(memUtil->GetActiveProcess(), address);
        if (funcRet.u32Ret <= INT_MAX)
        {
          funcRet.i32Ret = static_cast<int32_t>(funcRet.u32Ret);
        }
        else
        {
          funcRet.i32Ret = static_cast<int32_t>(funcRet.u32Ret - INT_MIN) + INT_MIN;
        }

        switch (function.Modifier)
        {
        case FMULT:
          funcRet.i32Ret = funcRet.i32Ret * modValue;
          break;
        case FDIV:
          funcRet.i32Ret = funcRet.i32Ret / modValue;
          break;
        case FADD:
          funcRet.i32Ret = funcRet.i32Ret + modValue;
          break;
        case FSUB:
          funcRet.i32Ret = funcRet.i32Ret - modValue;
          break;
        case CMP_EQ:
          funcRet.i32Ret = funcRet.i32Ret == modValue ? 1 : 0;
          break;
        case CMP_NE:
          funcRet.i32Ret = funcRet.i32Ret == modValue ? 0 : 1;
          break;
        case CMP_G:
          funcRet.i32Ret = funcRet.i32Ret > modValue ? 1 : 0;
          break;
        case CMP_GE:
          funcRet.i32Ret = funcRet.i32Ret >= modValue ? 1 : 0;
          break;
        case CMP_L:
          funcRet.i32Ret = funcRet.i32Ret < modValue ? 1 : 0;
          break;
        case CMP_LE:
          funcRet.i32Ret = funcRet.i32Ret <= modValue ? 1 : 0;
          break;
        }
      }
      break;
      // read[](size,address,ptr_offset1,ptr_offset2,...)
      case Interpreter::READ_ARRAY:
      {
        uintptr_t length = Interpreter::ToInteger(function.Parameters[0]);
        uintptr_t address = memUtil->MapAddress(function.Parameters[1]);
        if (2 < function.Parameters.size())
        {
          std::vector<uintptr_t> addresses(function.Parameters.size(), 0);
          addresses[0] = address;
          for (size_t i = 2; i < function.Parameters.size(); ++i)
          {
            addresses[i - 1] = Interpreter::ToInteger(function.Parameters[i]);
          }

          address = memUtil->GetPointerAddress(memUtil->GetActiveProcess(), addresses);
        }

        funcRet.arrRet = memUtil->ReadArray(memUtil->GetActiveProcess(), address, length);
      }
      break;
      // write8(value,address,ptr_offset1,ptr_offset2,...)
      case Interpreter::WRITE_8:
      case Interpreter::WRITE_8I:
      {
        std::vector<char> buf(sizeof(uint8_t), 0);
        uint8_t value = ToInteger(function.Parameters[0]);
        memcpy(buf.data(), &value, buf.size());

        uintptr_t address = memUtil->MapAddress(function.Parameters[1]);
        if (2 < function.Parameters.size())
        {
          std::vector<uintptr_t> addresses(function.Parameters.size() - 1, 0);
          addresses[0] = address;
          for (size_t i = 2; i < function.Parameters.size(); ++i)
          {
            addresses[i - 1] = Interpreter::ToInteger(function.Parameters[i]);
          }

          address = memUtil->GetPointerAddress(memUtil->GetActiveProcess(), addresses);
        }

        funcRet.bRet = memUtil->Write(memUtil->GetActiveProcess(), address, buf);
      }
      break;
      // write16(value,address,ptr_offset1,ptr_offset2,...)
      case Interpreter::WRITE_16:
      case Interpreter::WRITE_16I:
      {
        std::vector<char> buf(sizeof(uint16_t), 0);
        uint16_t value = ToInteger(function.Parameters[0]);
        memcpy(buf.data(), &value, buf.size());

        uintptr_t address = memUtil->MapAddress(function.Parameters[1]);
        if (2 < function.Parameters.size())
        {
          std::vector<uintptr_t> addresses(function.Parameters.size() - 1, 0);
          addresses[0] = address;
          for (size_t i = 2; i < function.Parameters.size(); ++i)
          {
            addresses[i - 1] = Interpreter::ToInteger(function.Parameters[i]);
          }

          address = memUtil->GetPointerAddress(memUtil->GetActiveProcess(), addresses);
        }

        funcRet.bRet = memUtil->Write(memUtil->GetActiveProcess(), address, buf);
      }
      break;
      // writeF(value,address,ptr_offset1,ptr_offset2,...)
      case Interpreter::WRITE_FLOAT:
      {
        std::vector<char> buf(sizeof(float), 0);
        float value = strtof(function.Parameters[0].c_str(), nullptr);
        memcpy(buf.data(), &value, buf.size());

        uintptr_t address = memUtil->MapAddress(function.Parameters[1]);
        if (2 < function.Parameters.size())
        {
          std::vector<uintptr_t> addresses(function.Parameters.size() - 1, 0);
          addresses[0] = address;
          for (size_t i = 2; i < function.Parameters.size(); ++i)
          {
            addresses[i - 1] = Interpreter::ToInteger(function.Parameters[i]);
          }

          address = memUtil->GetPointerAddress(memUtil->GetActiveProcess(), addresses);
        }

        funcRet.bRet = memUtil->Write(memUtil->GetActiveProcess(), address, buf);
      }
      break;
      // write32(value,address,ptr_offset1,ptr_offset2,...)
      case Interpreter::WRITE_32:
      case Interpreter::WRITE_32I:
      {
        std::vector<char> buf(sizeof(uint32_t), 0);
        uint32_t value = ToInteger(function.Parameters[0]);
        memcpy(buf.data(), &value, buf.size());

        uintptr_t address = memUtil->MapAddress(function.Parameters[1]);
        if (2 < function.Parameters.size())
        {
          std::vector<uintptr_t> addresses(function.Parameters.size() - 1, 0);
          addresses[0] = address;
          for (size_t i = 2; i < function.Parameters.size(); ++i)
          {
            addresses[i - 1] = Interpreter::ToInteger(function.Parameters[i]);
          }

          address = memUtil->GetPointerAddress(memUtil->GetActiveProcess(), addresses);
        }

        funcRet.bRet = memUtil->Write(memUtil->GetActiveProcess(), address, buf);
      }
      break;
      // write[8]](value,address,ptr_offset1,ptr_offset2,...)
      case Interpreter::WRITE_ARRAY:
      {
        std::vector<char> buf(function.Parameters[0].length(), 0);
        memcpy(buf.data(), function.Parameters[0].data(), buf.size());

        uintptr_t address = memUtil->MapAddress(function.Parameters[1]);
        if (2 < function.Parameters.size())
        {
          std::vector<uintptr_t> addresses(function.Parameters.size() - 1, 0);
          addresses[0] = address;
          for (size_t i = 2; i < function.Parameters.size(); ++i)
          {
            addresses[i - 1] = Interpreter::ToInteger(function.Parameters[i]);
          }

          address = memUtil->GetPointerAddress(memUtil->GetActiveProcess(), addresses);
        }

        funcRet.bRet = memUtil->Write(memUtil->GetActiveProcess(), address, buf);
      }
      break;
      // setflag(flag,val);
      case Interpreter::SET_FLAG:
      {
        DeveloperMode = ('1' == function.Parameters[1][0]);
      }
      break;
    }
  }

  return funcRet;
}