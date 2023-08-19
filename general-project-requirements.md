# MS Circuit Game System General Project Requirements

These are project requirements that are recommended for any game project
made based on this open-source project.

## Values provided to the user
- Video games that are able to be preserved through maintenance and repair of the circuitry
of the game system. 
- Modifiable games i.e. graphics, sound, and game logic can be modified by hardware or code.

## Requirements derived from values provided

### Circuit
- A functioning circuit schematic design validated by circuit simulation and/or models.
- Jumpers to allow hardware modification of the logic, graphics, sound of the game.

### PCB Design 
- Printed circuit board must be EMC (Electromagnetic compatibility) compliant.
- SMD jumpers to allow hardware modification of the logic, graphics, sound of the game.
- Test points for easy debug.
- Make pcb modules of the type pcb shield or pcb hat 
i.e. pcb that can be mounted to another pcb through connector.
For example, an raspberry pi hat connecting to a raspberry pi system pcb 
or Arduino shield connecting to an Arduino.

### Enclosure Form Factor
- Leave enough space for printed circuit board to fit.
- Make slots for the connectors for controllers and modifiable jumpers.
- Modular. Has holders that can hold
