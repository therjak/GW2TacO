#pragma once

class CFPUDouble
{
  uint32_t OriginalFPUState;

public:

  CFPUDouble();
  ~CFPUDouble();
};

class CFPUAnsiCRounding
{
  uint32_t OriginalFPUState;

public:

  CFPUAnsiCRounding();
  ~CFPUAnsiCRounding();
};
