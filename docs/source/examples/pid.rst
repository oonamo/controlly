PID Leader Follower
======================

.. raw:: html

   <div class="wasm-example-container">
      <iframe src="../_static/wasm/pid_leader_follower.html">
      </iframe>
   </div>

The Control Theory
----------------------

.. figure:: ../_static/pid_leader_follower.drawio.svg
   :align: center
   :alt: Block diagram of the PID Loop

Before applying physics to the follower, we must first calculate the corrective force.

.. math::
   e(t) = x_{target} - x_{current}

Applying a standard PID Controller:

.. math::
   F_{corrective} = K_p e(t) + K_i \int e(t) dt + K_d \frac{de(t)}{dt}

We can use the corrective force as the force exerted by the vehicle's engine:

.. math::
   F_{eng} = F_{corrective}

We model our system as a vehicle that exhibits an opposing drag force:

.. math::
   F = m\ddot{x} = F_{eng} - b\dot{x}

Solving for the acceleration:

.. math::
  \ddot{x} = \frac{F_{eng} - b\dot{x}}{m}

Applying Euler integration:

.. math::
   \begin{aligned}
   \dot{x}[k + 1] &= \dot{x}[k] + \ddot{x}[k] \Delta t \\
   x[k + 1] &= x[k] + \dot{x}[k] \Delta t
   \end{aligned}

Relevant Source Code
----------------------

.. literalinclude:: ../../../examples/raylib/pid_leader_follower/pid_leader_follower.c
   :linenos:
   :start-after: // [DOC_START: pid_example]
   :end-before: // [DOC_END: pid_example]
   :caption: PID Control Loop
