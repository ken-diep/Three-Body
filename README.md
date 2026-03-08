# Three-Body

Needs SFML 3.0 and C++ 17 to build.
Contains a two-body simulation prototype and three-body simulation.



Background:

I started by creating a two-body simulation using the Earth and the Sun. I wanted to explore different numerical methods of calculating orbital mechanics.



Semi-implicit Euler first calculates the velocity at time n+1, then uses that velocity to calculate the position at n+1.



Verlet calculates the new position using the current velocity and acceleration. It then calculates the new acceleration based on the new position, then the new velocity using the average of the old and new acceleration.



Further reading: 



https://ciechanow.ski/earth-and-sun/

