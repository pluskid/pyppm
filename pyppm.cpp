#include <cstdio>
#include <cstring>
#include <cerrno>
#include <Python.h>
#include "ppm_model.h"
#include "io_adapter.h"

using namespace std;

extern "C" {
    
typedef struct 
{
    PyObject_HEAD
    PPMModel *model;
} Model;

static void Model_dealloc(PyObject *self);
static PyObject * Model_GetAttr(PyObject *self, char *attrname);
    
static PyTypeObject Model_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "Model",
    sizeof(Model),
    0,
    (destructor)Model_dealloc,
    0,
    (getattrfunc)Model_GetAttr,
    /* rest are NULLs */
};

#define Model_Check(v) ((v)->ob_type == &Model_Type)
#define Model_Ptr(v)   (((Model *)(v))->model)

static PyObject *Model_New(PyObject *self, PyObject *args) 
{
    PPMModel *pm;
    Model *model = NULL;
    char *path = NULL;

    if (PyArg_ParseTuple(args, "|s", &path)) {
        if (path != NULL) {
            FILE *fp = fopen(path, "rb");
            if (fp == NULL) {
                PyErr_SetString(PyExc_IOError, strerror(errno));
                return (PyObject *)model;
            } else {
                pm = PPMModel::load(fp);
                fclose(fp);
            }
        } else {
            pm = new PPMModel();
        }        
        model = PyObject_NEW(Model, &Model_Type);
        model->model = pm;
    }

    return (PyObject *)model;
}

static void Model_dealloc(PyObject *self) 
{
    Model_Ptr(self)->decref();
    PyMem_DEL(self);
}

static PyObject *Model_dump(PyObject *self, PyObject *args) 
{
    char *path = NULL;
    
    if (PyArg_ParseTuple(args, "s", &path)) {
        FILE *fp = fopen(path, "wb");
        if (fp == NULL) {
            PyErr_SetString(PyExc_IOError, strerror(errno));
        } else {
            PPMModel::dump(Model_Ptr(self), fp);
            fclose(fp);
        }
    }
    
    return Py_BuildValue("");
}

static PyObject *Model_train(PyObject *self, PyObject *args)
{
    char *path = NULL;

    if (PyArg_ParseTuple(args, "s", &path)) {
        FILE *fp = fopen(path, "rb");
        if (fp == NULL) {
            PyErr_SetString(PyExc_IOError, strerror(errno));
        } else {
            NullOutputAdapter nad;

            PPMEncoder<NullOutputAdapter, DefaultContextUpdater> penc(nad, Model_Ptr(self));
            penc.start_encoding();
            for (int ch = fgetc(fp); ch != EOF; ch = fgetc(fp)) {
                penc.encode(ch);
            }
            penc.finish_encoding();
            fclose(fp);
            
            return Py_BuildValue("i", nad.count());
        }
    }
    return Py_BuildValue("");
}

static PyObject *Model_predict(PyObject *self, PyObject *args)
{
    char *path = NULL;

    if (PyArg_ParseTuple(args, "s", &path)) {
        FILE *fp = fopen(path, "rb");
        if (fp == NULL) {
            PyErr_SetString(PyExc_IOError, strerror(errno));
        } else {
            NullOutputAdapter nad;

            PPMModel *pm = Model_Ptr(self);
            pm->m_buffer.reset();
            PPMEncoder<NullOutputAdapter, NopeContextUpdater> penc(nad, pm);
            penc.start_encoding();
            for (int ch = fgetc(fp); ch != EOF; ch = fgetc(fp))
                penc.encode(ch);
            penc.finish_encoding();
            fclose(fp);

            return Py_BuildValue("i", nad.count());
        }
    }
    return Py_BuildValue("");
}

static PyMethodDef Model_methods[] = {
    {"dump", Model_dump, METH_VARARGS},
    {"train", Model_train, METH_VARARGS},
    {"predict", Model_predict, METH_VARARGS},
    {NULL, NULL},
};

static PyObject * Model_GetAttr(PyObject *self, char *attrname) 
{
    return Py_FindMethod(Model_methods, self, attrname);
}
    
static PyMethodDef methods[] = {
    {"Model", Model_New, METH_VARARGS},
    {NULL, NULL},
};

void initpyppm() 
{
    Py_InitModule("pyppm", methods);
}

} // extern "C"
