#include <Python.h>
#include "api.h"

// static PyObject*
// emb_numargs(PyObject *self, PyObject *args)
// {
//     if(!PyArg_ParseTuple(args, ":numargs"))
//         return NULL;
//     return Py_BuildValue("i", numargs);
// }

Python::API& Python::API::getInstance()
{
    static Python::API instance;
    return instance;
}

Python::API(){
  Py_Initialize();
}

Python::~API(){
  Py_Finalize();
}

Python::apply_func_to_buffer(float* buf, int h, int w, std::string py_code){
  
}