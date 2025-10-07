#include <cassert>

#include "mstring.h"

// Color code: Elevators
// - Green: Door open - Available
// - Magenta: Moving

// Color code: People 
// - Yellow: Going somewhere
// - White: Idle
// - Red (Residents only): Doing errand
// - Cyan (Visitors only): Visitor

/////////////////////////////////////////////////////////////////////////////
/// SIMULATION PARAMETERS ///////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#define RANDOM_RESIDENTS 1
#define RANDOM_VISITORS 1

#define RANDOM_PARAMETERS 1

#define NUM_BUILDINGS 8
#define NUM_VISITORS 5
#define NUM_RESIDENTS 20

#define DUMP 0

//For present simulations, set following values and set RANDOM_PARAMETERS to 0:

int building_heights[NUM_BUILDINGS] = {5,18,12,19,8,8,5,10};
int building_num_elevators[NUM_BUILDINGS] = {1,4,2,3,2,2,1,3};

int resident_addresses[NUM_RESIDENTS];
int resident_floors[NUM_RESIDENTS];

/////////////////////////////////////////////////////////////////////////////

Building* buildings[NUM_BUILDINGS];
Resident* residents[NUM_RESIDENTS];
Visitor* visitors[NUM_VISITORS];
Screen* s;

int random_errand_proc();
int random_visit_proc();
int random_building();
int random_floor(int address);

/////////////////////////////////////////////////////////////////////////////

//////////////////////////////
// Elevator 
/////////////////////////////

/* Graphics: Update symbol and color */
void Elevator::update_movement() {
    if (destination < currentFloor) {
        direction = (char*)downarrow;
        c = magenta;
    }
    if (destination > currentFloor) {
        direction = (char*)uparrow;
        c = magenta;
    }
}

int Elevator::get_floor() {return currentFloor;}
char *Elevator::get_direction() {return direction;}
int Elevator::get_destination() {return destination;}
int Elevator::is_available() {return isAvailable;}
color Elevator::get_color() {return c;}

void Elevator::set_destination(int floor) {destination=floor;}

void Elevator::set_passenger(Person *person) {
    passenger = person;
    isAvailable = 0;
}

/* Liberate elevator - deallocate from passenger */
void Elevator::free() {
    c = green;
    direction = (char*)dooropen;
    isAvailable = 1;
    passenger = NULL;
}

void Elevator::update() {

    update_movement();

    if (direction == (char*)uparrow) {
        if (currentFloor < building_nFloors) {++currentFloor;}
    }
    if (direction == (char*)downarrow) {
        if (currentFloor > 0) {--currentFloor;}
    }
}

Elevator::Elevator (int floor, char *arrow, int nFloors) {
    assert(floor>=0 && building_nFloors >= floor);
    assert(arrow == uparrow || arrow == downarrow || arrow == notmoving || arrow == dooropen);
    currentFloor = floor;
    direction = arrow;
    building_nFloors = nFloors;
    isAvailable = 1;
    c = green;
}

//////////////////////////////
// Floor 
/////////////////////////////

int Floor::get_floor() {return level;}
Building* Floor::get_building() {return building;}

void Floor::call_elevator(Person *person) {
    Elevator *elevator = building->get_nearest_available_elevator(level);
    if (elevator == NULL) {
        return;
    }
    elevator->set_passenger(person);
    elevator->set_destination(level);
    person->set_elevator(elevator);
}

Floor::Floor (int num_floor, Building *building_ref) {
    level = num_floor;
    building = building_ref;
}

//////////////////////////////
// Building  
/////////////////////////////

void Building::show_building() {
    s->move(extraLeftOffset+LEFTOFFSET-1,BOTTOMOFFSET);
    s->bg(cyan);
    s->fg(white);
    cout << to_string(address);
    s->bg(white);
    s->column(".", nFloors+2);
    s->row(".", nColumns+1);
    s->move(extraLeftOffset+LEFTOFFSET+nColumns,BOTTOMOFFSET);
    s->column(".",nFloors+2);
    
    s->reset();
    s->move(0,0);
    s->bg(black);
}

void Building::show_elevators() {
        s->fg(white);

        Elevator *elevator;

        for (int i=0; i<nElevators; i++) {
            elevator = elevators[i];
            /* Place destination marker*/
            if (!(elevator->is_available())) {
                s->move(LEFTOFFSET+elevator->x,BOTTOMOFFSET+elevator->get_destination());
                s->bg(black);
                s->fg(red);
                s->row(destination_marker, 1);
            }
            s->move(LEFTOFFSET+elevator->x,BOTTOMOFFSET+elevator->get_floor());
            s->fg(white);
            s->bg(elevator->get_color());
            char *direction = elevator->get_direction();
            s->row(direction, 1);
        }
        s->reset();
        s->move(0,0);
    }


/* Return the closest available elevator */
Elevator *Building::get_nearest_available_elevator(int requested_floor) {

    /* Create list of distances */
    Elevator *elevator;
    int distances[nElevators];

    int first_free = -1;
    int first_free_set = 0;
    
    for (int i=0; i<nElevators; i++) {

        elevator = elevators[i];

        if (!(first_free_set)) {
            if (elevator->is_available()) {
                first_free = i;
                first_free_set = 1;
            }
        }
        
        distances[i] = abs(elevator->get_floor() - requested_floor);
    }

    /* No elevator available ; wait for next update.*/
    if (!(first_free_set)) {
        return NULL;
    }

    /* Get closest available elevator */
    int i_min=first_free;
    for (int i=first_free; i<nElevators; i++) {
        if (distances[i]<distances[i_min]) {
            if (elevators[i]->is_available()) {
                i_min = i;
            }
        }
    }

    return elevators[i_min];

}

Floor *Building::get_floor_from_int(int i) {return floors[i];}

Screen *Building::get_screen() {return s;}

int Building::get_num_columns() {return nColumns;}


void Building::update() {

    /* Update elevators */
    Elevator *elevator;

    for (int i=0; i<nElevators; i++) {
        elevator = elevators[i];
        elevator->update();
    }

    /* Display building */
    show_building();
    show_elevators();
}    

void Building::dump() {
    printf("Building information:\n");
    printf(" * number of floors: %d\n",nFloors);
    printf(" * number of elevators: %d\n",nElevators);
}

void Building::elevators_dump() {
    printf("Elevator information:\n");
    for (int i=0; i<nElevators;i++) {
        int is_free = elevators[i]->is_available();
        printf(" * floor: %d - is_free: %d - destination: %d\n",elevators[i]->get_floor(),is_free, (is_free) ? -1 : elevators[i]->get_destination());
    }
}

void Building::set_next_building(Building *building) {nextBuilding = building;}
void Building::set_previous_building(Building *building) {
    previousBuilding = building;
    extraLeftOffset = previousBuilding->extraLeftOffset + previousBuilding->nColumns + BUILDING_GAP_SPACE;

    for (int i=0; i<nElevators ; i++) {
        elevators[i]->x = extraLeftOffset + 2*i + 2;
    }

    previousBuilding->set_next_building(this);
}

Building::Building (int num_floors, int num_elevators, 
    int building_address, Screen *screen) 
{
    assert(num_elevators<=MAX_ELEVATOR_NUM);
    assert(num_floors < MAX_FLOORS);

    nFloors = num_floors;
    nColumns = 2*num_elevators + 2;
    nElevators = num_elevators;

    extraLeftOffset = 0;
    address = building_address;

    /* Init elevators */
    for (int i=0; i<nElevators ; i++) {
        elevators[i] = new Elevator(0,(char*)dooropen, nFloors);
        elevators[i]->x = 2*i+2;
    }

    /* Init floors */
    for (int i=0; i<nFloors; i++) {
        floors[i] = new Floor(i,this);
    }

    /* Display */
    s = screen;
}

//////////////////////////////
// Person 
/////////////////////////////

int Person::get_destination() {return destination;}
int Person::is_in_elevator() {return inElevator;}
int Person::is_idle() {return idle;}

int Person::is_at_destination() {
    return (final_destination_building == building->address) && (final_destination_floor == floor->get_floor());
}

Elevator *Person::get_elevator() {return elevator;}
int Person::get_floor() {return floor->get_floor();}
Building *Person::get_building() {return building;}

// void Person::next_step() {

//     idle = 0;

//     /* Am I in the right building? */
//     if (final_destination_building == building->address) {
//         /* Am I on the right floor? */
//         if (final_destination_floor == floor->get_floor()) {
//             idle = 1;
//             return;
//         }
//         destination = final_destination_floor;
//         return;
//     }
//     /* Am I on the ground floor? */
//     if (floor->get_floor() == 0) {
//         /* Do I exit left or right? */
//         if (final_destination_building < building->address) {
//             if (building->previousBuilding == NULL) {
//                 printf("Invalid address. Becoming idle.\n");
//                 idle = 1;
//                 return;
//             }
//             isBetweenBuildings = -1;
//             return;
//         }
//         if (building->nextBuilding == NULL) {
//             printf("Invalid address. Becoming idle.\n");
//             idle = 1;
//             return;
//         }
//         isBetweenBuildings = +1;
//         return;
//     }
//     /* Go to ground floor. */
//     destination = 0;
//     return;
// }

void Person::show() {
    if (inElevator) {
        return;
    }
    if (idle) {
        s->fg(white);
    } else {
        s->fg(yellow);
    }
    s->bg(black);

    s->move(LEFTOFFSET+x+1, BOTTOMOFFSET+floor->get_floor());
    s->row(person_marker, 1);

    s->reset();
    s->move(0,0);
}

void Person::set_elevator(Elevator *allocated_elevator) {elevator = allocated_elevator;}

void Person::update_movement() {

    if (isBetweenBuildings<0) {
        /* moving left */
        Building *previous = building->previousBuilding;
        if (x <= (previous->extraLeftOffset + previous->get_num_columns()) ) {
            x = previous->extraLeftOffset + previous->get_num_columns() -2;
            building = previous;
            floor = building->get_floor_from_int(0);
            isBetweenBuildings = 0;
            return;
        }
        x -= 2;
        return;
    }
    if (isBetweenBuildings>0) {
        /* moving right */
        Building *next = building->nextBuilding;
        if (x >= (next->extraLeftOffset)) {
            x = next->extraLeftOffset;
            building = next;
            floor = building->get_floor_from_int(0);
            isBetweenBuildings = 0;
            return;
        }
        x += 2;
        return;
    }

    /* Am I trying to go somewhere? */
    if (!idle && !inElevator) {
        /* Am I in the right building? */
        if (final_destination_building != building->address) {
            if (floor->get_floor() != 0) {
                destination = 0;
            }
        }
        /* Has an elevator been allocated to me? */
        if (elevator == NULL) {
            floor->call_elevator(this);
            return;
        }
        /* Has the elevator arrived? Is the door opened? */
        if (elevator->get_floor() == floor->get_floor()) {

            /* Get in elevator */
            idle = 1;
            inElevator = 1;
            elevator->set_destination(destination);

            return;
        }
        return;
    }
    /* Am I in an elevator? */
    if (inElevator) {
        floor = building->get_floor_from_int(elevator->get_floor());

        /* Has the elevator arrived? */
        if (elevator->get_floor() == destination) {
            /* Get out of elevator. */
            inElevator = 0;
            elevator->free();
            elevator = NULL;

            return;
        }
    }
}

void Person::proc_final_destination(int destination_building, int destination_floor) {
    final_destination_building = destination_building;
    final_destination_floor = destination_floor;
}

Person::Person(Building *current_building, int current_floor) {
    building = current_building;
    s = building->get_screen();
    floor = building->get_floor_from_int(current_floor);
    inElevator = 0;
    idle = 1;
    isBetweenBuildings = 0;
    x = building->extraLeftOffset + 2;
}

//////////////////////////////
// Residents 
/////////////////////////////
Resident::Resident (Building *h_building, int h_floor, string identifier) {
    assert(h_building != NULL);
    building = h_building;
    s = building->get_screen();
    floor = building->get_floor_from_int(h_floor);
    inElevator = 0;
    idle = 1;
    isBetweenBuildings = 0;

    x = building->extraLeftOffset;

    onErrand = 0;

    name = identifier;
    
    home_building = building->address;
    home_floor = h_floor;
    final_destination_building = home_building;
    final_destination_floor = home_floor;
    destination = home_floor;
}

void Resident::proc_errand(int destination_building, int destination_floor) {
    final_destination_building = destination_building;
    final_destination_floor = destination_floor;
    idle = 0;
    onErrand = 1;
}

void Resident::go_home() {
    final_destination_building = home_building;
    final_destination_floor = home_floor;
    idle = 0;
    onErrand = 0;
}

int Resident::is_at_home() {
    return ( (building->address == home_building) && (floor->get_floor() == home_floor) );
}

void Resident::next_step() {

    /* Am I in the right building? */
    if (final_destination_building == building->address) {
        /* Am I on the right floor? */
        if (final_destination_floor == floor->get_floor()) {
            
            if (onErrand) {
                /* Doing errand */
                if (errand_tick_counter > RESIDENT_ERRAND_TIME) {
                    errand_tick_counter = 0;
                    onErrand = 0;
                    go_home();
                } else {
                    errand_tick_counter++;
                }
                return;

            } else { 
                /* At home */
                idle = 1;
                return;
            }
        }
        /* Right building, wrong floor */
        destination = final_destination_floor;
        return;
    }

    /* Am I on the ground floor? */
    if ( floor->get_floor() == 0 && (!idle) ) {
        /* Do I exit left or right? */
        if (final_destination_building < building->address) {

            assert(building->previousBuilding != NULL);
            isBetweenBuildings = -1;
            return;
        }

        assert(building->nextBuilding != NULL); 
        isBetweenBuildings = +1;
        return;
    }

    /* Go to ground floor to exit building. */
    destination = 0;
    return;
}

void Resident::show() {
    if (inElevator) {
        return;
    }
    if (idle) {
        s->fg(white);
    } else if (errand_tick_counter == 0) {
        s->fg(yellow);
    } else {
        s->fg(red);
    }
    s->bg(black);
    s->move(1+LEFTOFFSET+x,BOTTOMOFFSET+floor->get_floor());
    s->row(person_marker, 1);
    s->reset();
    s->move(0,0);
}

void Resident::update() {

    if (inElevator) {
        update_movement();

        if (!inElevator) {
            next_step();
        }
        return;
    }
    
    if (isBetweenBuildings != 0) {
        update_movement();
        
        if (isBetweenBuildings == 0) {
            next_step();
        }
        return;
    }

    if (!is_at_destination()) {
        idle = 0;
        next_step();
        update_movement();
        return;
    }

    if (is_at_home()) {
        x = building->extraLeftOffset;
        idle = 1;
        if (RANDOM_RESIDENTS) {
            if (random_errand_proc()) {
                int rand_building = random_building();
                proc_errand(rand_building, random_floor(rand_building));
            }
        }
        return;
    }

    if (is_at_destination()) {
        next_step();
    }
}

void Resident::dump() {
    printf("Resident information : ");
    cout << name << endl;
    printf(" * idle: %d - at destination: %d\n",idle, is_at_destination()); 
    printf(" * address: %d - floor: %d\n", home_building, home_floor);
    printf(" * going to: building: %d - floor: %d \n", final_destination_building, final_destination_floor);
    printf(" * destination in building: %d\n", destination);
}

//////////////////////////////
// Visitors 
/////////////////////////////
Visitor::Visitor (int destination_building, int destination_floor) {
    building = leftmost_building;
    assert(building != NULL);
    s = building->get_screen();
    floor = building->get_floor_from_int(0);
    inElevator = 0;
    idle = 1;
    isBetweenBuildings = 0;
    exit_protocol = 0;
    onVisit = 0;
    visit_tick_counter = 0;

    final_destination_building = destination_building;
    final_destination_floor = destination_floor;
}

void Visitor::leave() {
    final_destination_building = leftmost_building->address;
    final_destination_floor = 0;
    exit_protocol = 1;
    onVisit = 0;
}

void Visitor::proc_visit(int destination_building, int destination_floor) {
    final_destination_building = destination_building;
    final_destination_floor = destination_floor;
    idle = 0;
    onVisit = 1;
}

void Visitor::update() {

    if (inElevator) {
        update_movement();

        if (!inElevator) {
            idle = 0;
            next_step();
        }
        return;
    }
    
    if (isBetweenBuildings != 0) {
        update_movement();
        
        if (isBetweenBuildings == 0) {
            next_step();
        }
        return;
    }

    if (!is_at_destination()) {
        idle = 0;
        next_step();
        update_movement();
    }

    if (idle && (!inElevator)) {
        if (RANDOM_VISITORS) {
            if (random_visit_proc()) {
                int rand_building = random_building();
                proc_visit(rand_building, random_floor(rand_building));
            }
        }
        return;
    }

    if ((!onVisit) && is_at_destination()) {
        idle = 1;
    }

    if (is_at_destination()) {
        next_step();
    }
}

void Visitor::next_step() {

    /* Am I in the right building? */
    if (final_destination_building == building->address) {
        /* Am I on the right floor? */
        if (final_destination_floor == floor->get_floor()) {

            if (onVisit) {
                /* Visiting */
                if (visit_tick_counter > VISITOR_VISIT_TIME) {
                    visit_tick_counter = 0;
                    onVisit = 0;
                    leave();
                } else {
                    visit_tick_counter++;
                }
                return;
            } 
        }
        destination = final_destination_floor;
        return;
    }
    /* Am I on the ground floor? */
    if (floor->get_floor() == 0 && (!idle)) {
        /* Do I exit left or right? */
        if (final_destination_building < building->address) {
            assert(building->previousBuilding != NULL);
            isBetweenBuildings = -1;
            return;
        }
        assert(building->nextBuilding != NULL); 
        isBetweenBuildings = +1;
        return;
    }
    /* Go to ground floor. */
    destination = 0;
    return;
}

void Visitor::show() {
    if (inElevator) {
        return;
    }
    if (idle) {
        return;
    }
    s->fg(cyan);
    s->bg(black);
    s->move(1+LEFTOFFSET+x,BOTTOMOFFSET+floor->get_floor());
    s->row(person_marker, 1);
    s->reset();
    s->move(0,0);
}

void Visitor::dump() {}

//////////////////////////////
// Random generators 
/////////////////////////////

int random_errand_proc() {
    int n = myrand(RESIDENT_CHANCE_OF_ERRAND);
    return (n==0);
}
int random_visit_proc() {
    int n = myrand(VISITOR_CHANCE_OF_VISIT);
    return (n==0);
}
int random_building() {
    return (myrand(NUM_BUILDINGS)+1);
}
int random_floor(int address) {
    return (myrand(building_heights[address-1]-1));
}

//////////////////////////////
// Execution 
/////////////////////////////

void init() {

    /* Random parameters */
    if (RANDOM_PARAMETERS) {
        for (int i=0; i<NUM_BUILDINGS; i++) {
            building_heights[i] = 5+myrand(MAX_FLOORS-5);
            building_num_elevators[i] = 1+myrand(MAX_ELEVATOR_NUM-1);
        }
        for (int i=0; i<NUM_RESIDENTS; i++) {
            resident_addresses[i] = 1+myrand(NUM_BUILDINGS);
            resident_floors[i] = 1+myrand(building_heights[resident_addresses[i]-1] -1);
        }
    }
    
    /* Initialise Screen */
    s = new Screen(BOTTOMOFFSET+5+MAX_FLOORS);
    srand48(time(NULL));
    s->clear();
    if (DUMP) {
        printf("Initialised screen.\n");
    }
    
    /* Initialise Buildings */
    for (int b=0; b<NUM_BUILDINGS; b++) {
        buildings[b] = new Building(building_heights[b], building_num_elevators[b], b+1, s);
        if (b>0) {
            buildings[b]->set_previous_building(buildings[b-1]);
        }
    }
    if (DUMP) {
        printf("Initialised buildings.\n");
    }
    
    leftmost_building = buildings[0];

    /* Initialise Residents */
    for (int r=0; r<NUM_RESIDENTS; r++) {
        residents[r] = new Resident(buildings[resident_addresses[r]-1],resident_floors[r],name_list[r]);
    }
    if (DUMP) {
        printf("Initialised residents.\n");
    }

    /* Initialise Visitors */
    for (int v=0; v<NUM_VISITORS; v++) {
        visitors[v] = new Visitor(1,0);
    }
    if (DUMP) {
        printf("Initialised visitors.\n");
    }
}

void update() {

    s->clear();
    s->reset();
    s->move(0,0);

    /* Update Buildings */
    for (int b=0; b<NUM_BUILDINGS; b++) {
        buildings[b]->update();
    }
    
    /* Update Residents */
    for (int r=0; r<NUM_RESIDENTS; r++) {
        residents[r]->update();
        residents[r]->show();
    }

    /* Update Visitors */
    for (int v=0; v<NUM_VISITORS; v++) {
        visitors[v]->update();
        visitors[v]->show();
    }

    /* (Optional) Dump */
    if (DUMP) {
        for (int r=0; r<NUM_RESIDENTS; r++) {
            residents[r]->dump();
        }
        for (int v=0; v<NUM_VISITORS; v++) {
            visitors[v]->dump();
        }
    }
}

void loop() {
    for (int it=0;it<MAX_ITERATIONS;it++) {
        mysleep(FRAMERATE);
        update();
        printf("Iteration %d complete.\n",it);
    }
}

int main() {

    init();
    if (DUMP) {
        printf("Initialised.\n");
    }
    loop();

    return 0;
}
