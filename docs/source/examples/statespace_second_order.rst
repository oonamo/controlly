State Space Second Order
=========================

.. raw:: html

   <div class="wasm-example-container">
      <iframe src="../_static/wasm/statespace_second_order.html">
      </iframe>
   </div>

The Control Theory
----------------------

This examples models a standard continous-time, second-order transfer function. The dynamics of this systems are defined by only two parameters: the natural frequency (:math:`\omega_n`) and the damping ratio (:math:`\zeta`).

The standard second-order transfer function is modeled as so:

.. math::
   H(s) = \frac{\omega_n^2}{s^2 + 2s\zeta \omega_n + \omega_n^2}

In the `ControlSetup()`, function we define the rational transfer function as:

* **Numerator(n)**: [:math:`\omega_n^2`]
* **Denominator(n)**: [:math:`1, 2\zeta \omega_n, \omega_n^2`]

To map this into the standard state-space equations:

.. math::
   \begin{aligned}
    \dot{x} &= Ax + Bu \\
    y &= Cx + Du \\
   \end{aligned}

Converting to State Space
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The :cpp:func:`Control_StateSpace_FromTF()` is called, the library automatically translates the transfer function into *Controllable Canonical Form (CCF)*.

Because the numerator is a constant (degree 0) and has no :math:`s` terms, the resulting :math:`C` matrix is greatly simplified. The continuous-time state-space matrices are modeled as follows:

.. math::
   \begin{aligned}
     A &= \begin{bmatrix} 0 & 1 \\ -\omega_n^2 & -2\zeta \omega_n \end{bmatrix}
     &&B = \begin{bmatrix} 0 \\ 1 \end{bmatrix} \\
     C &= \begin{bmatrix} \omega_n^2 & 0 \end{bmatrix}
     &&D = \begin{bmatrix} 0 \end{bmatrix}
   \end{aligned}

If using :cpp:func:`Control_StateSpace_FromTF()`, the state space model is stored in persistent memory (The scratch arena is free to clear).

Initial States
^^^^^^^^^^^^^^^^^^^^^^^^^^^
The CCF tracks internal states rather then direct outputs. To correctly specify a visual starting position we must reverse-engineer the :math:`C` matrix.

The output equation is:

.. math::
   y = Cx = \begin{bmatrix}\omega_n^2 & 0\end{bmatrix} \begin{bmatrix}x_1 \\ x_2 \end{bmatrix}

Evaluating this simple matrix multiplication we get the system output as :math:`y=\omega_n^2 x_1`


Relevant Source Code
----------------------

.. literalinclude:: ../../../examples/raylib/statespace_second_order/statespace_second_order.c
   :linenos:
   :start-after: // [DOC_START: statespace_example]
   :end-before: // [DOC_END: statespace_example]
   :caption: State Space Loop
