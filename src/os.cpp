#include "python.h"

#define Py_NO_LINK_LIB
#ifdef _DEBUG
#undef _DEBUG
#define EMBEDDITYS_RESTORE_DEBUG
#endif
#include <Python.h>
#ifdef EMBEDDITYS_RESTORE_DEBUG
#define _DEBUG
#undef EMBEDDITYS_RESTORE_DEBUG
#endif
#include <stdlib.h>
#include <string.h>

static char* copy_string(const char* value)
{
	size_t length;
	char* result;

	if (value == NULL) value = "";
	length = strlen(value);
	result = (char*)malloc(length + 1);
	if (result == NULL) return NULL;
	memcpy(result, value, length + 1);
	return result;
}

static char* python_exception_text(void)
{
	PyObject* type = NULL;
	PyObject* value = NULL;
	PyObject* traceback = NULL;
	PyObject* text = NULL;
	const char* utf8;
	char* result;

	PyErr_Fetch(&type, &value, &traceback);
	PyErr_NormalizeException(&type, &value, &traceback);
	text = PyObject_Str(value != NULL ? value : type);
	utf8 = text != NULL ? PyUnicode_AsUTF8(text) : NULL;
	result = copy_string(utf8 != NULL ? utf8 : "Python execution failed");

	Py_XDECREF(type);
	Py_XDECREF(value);
	Py_XDECREF(traceback);
	Py_XDECREF(text);
	return result;
}

static int add_safe_builtin(PyObject* builtins, const char* name)
{
	PyObject* value = PyEval_GetBuiltins();
	PyObject* builtin = PyDict_GetItemString(value, name);
	return builtin != NULL && PyDict_SetItemString(builtins, name, builtin) == 0;
}

int python_run_sandboxed(const char* source, char** output, char** error)
{
	PyObject* globals = NULL;
	PyObject* builtins = NULL;
	PyObject* io = NULL;
	PyObject* string_io_type = NULL;
	PyObject* string_io = NULL;
	PyObject* old_stdout = NULL;
	PyObject* output_text = NULL;
	const char* output_utf8;
	PyGILState_STATE gil;
	static const char* safe_builtins[] = {
		"abs", "all", "any", "bool", "dict", "enumerate", "filter", "float",
		"int", "len", "list", "map", "max", "min", "print", "range", "repr",
		"reversed", "round", "set", "slice", "sorted", "str", "sum", "tuple", "zip"
	};
	size_t i;
	int result = 1;

	if (output == NULL || error == NULL || source == NULL) return 2;
	*output = NULL;
	*error = NULL;

	if (!Py_IsInitialized()) Py_Initialize();
	if (!Py_IsInitialized()) {
		*error = copy_string("unable to initialize Python");
		return 2;
	}
	gil = PyGILState_Ensure();

	globals = PyDict_New();
	builtins = PyDict_New();
	if (globals == NULL || builtins == NULL) {
		*error = python_exception_text();
		goto cleanup;
	}

	for (i = 0; i < sizeof(safe_builtins) / sizeof(safe_builtins[0]); ++i) {
		if (!add_safe_builtin(builtins, safe_builtins[i])) {
			*error = copy_string("unable to configure Python sandbox");
			goto cleanup;
		}
	}
	if (PyDict_SetItemString(globals, "__builtins__", builtins) != 0) {
		*error = python_exception_text();
		goto cleanup;
	}

	io = PyImport_ImportModule("io");
	if (io == NULL) {
		*error = python_exception_text();
		goto cleanup;
	}
	string_io_type = PyObject_GetAttrString(io, "StringIO");
	string_io = string_io_type != NULL ? PyObject_CallObject(string_io_type, NULL) : NULL;
	if (string_io == NULL) {
		*error = python_exception_text();
		goto cleanup;
	}

	old_stdout = PySys_GetObject("stdout");
	if (PySys_SetObject("stdout", string_io) != 0) {
		*error = python_exception_text();
		goto cleanup;
	}

	if (PyRun_StringFlags(source, Py_file_input, globals, globals, NULL) == NULL) {
		PySys_SetObject("stdout", old_stdout);
		*error = python_exception_text();
		goto cleanup;
	}

	PySys_SetObject("stdout", old_stdout);
	output_text = PyObject_CallMethod(string_io, "getvalue", NULL);
	output_utf8 = output_text != NULL ? PyUnicode_AsUTF8(output_text) : NULL;
	*output = copy_string(output_utf8 != NULL ? output_utf8 : "");
	if (*output == NULL) {
		*error = copy_string("unable to allocate Python output");
		goto cleanup;
	}
	result = 0;

cleanup:
	if (result != 0 && *error == NULL) *error = copy_string("Python sandbox execution failed");
	Py_XDECREF(output_text);
	Py_XDECREF(string_io);
	Py_XDECREF(string_io_type);
	Py_XDECREF(io);
	Py_XDECREF(builtins);
	Py_XDECREF(globals);
	PyGILState_Release(gil);
	return result;
}

void python_free_buffer(char* buffer)
{
	free(buffer);
}
