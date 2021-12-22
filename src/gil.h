//
// Taken from the pybind11 project: https://github.com/pybind/pybind11/blob/14976c853b8422dc41ce7874742cab122684c97e/include/pybind11/gil.h
//
//Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>, All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//        modification, are permitted provided that the following conditions are met:
//
//1. Redistributions of source code must retain the above copyright notice, this
//list of conditions and the following disclaimer.
//
//2. Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation
//and/or other materials provided with the distribution.
//
//3. Neither the name of the copyright holder nor the names of its contributors
//        may be used to endorse or promote products derived from this software
//        without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//        ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//        FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//        DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//        SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//        CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//Please also refer to the file .github/CONTRIBUTING.md, which clarifies licensing of
//external contributions to this project including patches, pull requests, etc.

#pragma once
#include <Python.h>

class gil_scoped_release {
public:
    explicit gil_scoped_release()  {
        // `get_internals()` must be called here unconditionally in order to initialize
        // `internals.tstate` for subsequent `gil_scoped_acquire` calls. Otherwise, an
        // initialization race could occur as multiple threads try `gil_scoped_acquire`.
        tstate = PyEval_SaveThread();
    }
//
//    gil_scoped_release(gil_scoped_release&& other) noexcept
//        : tstate(other.tstate), active(other.active) {
//        other.tstate = nullptr;
//        other.active = false;
//    }
//
//    gil_scoped_release& operator=(gil_scoped_release&& other) noexcept {
//        tstate = other.tstate;
//        other.tstate = nullptr;
//        active = other.active;
//        other.active = false;
//        return *this;
//    }

    /// This method will disable the PyThreadState_DeleteCurrent call and the
    /// GIL won't be acquired. This method should be used if the interpreter
    /// could be shutting down when this is called, as thread deletion is not
    /// allowed during shutdown. Check _Py_IsFinalizing() on Python 3.7+, and
    /// protect subsequent code.
    void disarm() {
        active = false;
    }

    ~gil_scoped_release() {
        if (!tstate)
            return;
        // `PyEval_RestoreThread()` should not be called if runtime is finalizing
        if (active)
            PyEval_RestoreThread(tstate);
    }
private:
    PyThreadState *tstate;
    bool active = true;
};