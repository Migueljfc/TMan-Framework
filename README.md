<p align = "left"> SOTR Project 2022 </p>
The objective of the work is developing a framework (Task Manager – TMan) that allows registering a
set of FreeRTOS tasks, associate each task with a set of attributes (e.g. period, deadline, phase,
precedence constraints) and activate those tasks at the appropriate instants.
<br></br>
<p align = "left">• TMAN_Init: initialization of the framework </p>
<p align = "left">• TMAN_Close: terminate the framework </p>
<p align = "left">• TMAN_TaskAdd: Add a task to the framework </p>
<p align = "left">• TMAN_TaskRegisterAttributes: Register attributes (e.g. period, phase, deadline, precedence
constraints) for a task already added to the framework </p>
<p align = "left">• TMAN_TaskWaitPeriod: Called by a task to signal the termination of an instance and wait for
the next activation </p>
<p align = "left">• TMAN_TaskStats: returns statistical information about a task. Provided information must
include at least the number of activations, but additional info (e.g. number of deadline misses)
will be valued. </p>

