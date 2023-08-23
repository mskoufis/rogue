
#include "rogue/PythonError.h"

namespace rogue { 

const char* PythonError::what() const noexcept { 
  PyObject* ptype{nullptr}; 
  PyObject* pvalue{nullptr}; 
  PyObject* ptraceback{nullptr}; 

  PyErr_Fetch(&ptype, &pvalue, &ptraceback); 
  if (ptype == nullptr) { 
    throw std::runtime_error("A python error was thrown but the ptype has been "
		             "set to nullptr."); 
  }
  
  PyErr_NormalizeException(&ptype, &pvalue, &ptraceback); 

}
}
