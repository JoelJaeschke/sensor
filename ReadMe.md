# Lacktivity - Sensor Network
This piece of soft- and hardware will aid in our study to determine effective ways of increasing physical activity
at work. We will investigate multiple measures, for example:
 - Competition between multiple locations to encourage more walking
 - Better user experience when taking the stairs (interesting information on the way, interactive art)
 - Dettering posters (e.g. People who do not take the stairs life more unhealthy lifes)
 - Motivational posters (sixth floor equals 1.6% of the Zugspitze)
We therefor need a realiable and cheap way to track the number of people using stairs. Commerically available solutions
are too expensive and also do not fit our use case quite right, so we set out to do it ourselfes.

## ToDo
- [x] Basic architectural considerations
- [x] First working prototype
- [x] Debounce receiver signal to avoid triggering interrupts multiple times
- [] Add persistent storage facility
    -> Change partition table
- [] Mesh networking between multiple sensors