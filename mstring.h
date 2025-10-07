#include "screen.cpp"

#define LEFTOFFSET 10
#define BOTTOMOFFSET 2
#define BUILDING_GAP_SPACE 10

#define MAX_ELEVATOR_NUM 5
#define MAX_FLOORS 20
#define MAX_NAME_LENGTH 4

#define FRAMERATE 100
#define MAX_ITERATIONS 1000

#define RESIDENT_ERRAND_TIME 5
#define VISITOR_VISIT_TIME 10


/* For COVID-19 mode: multiply values by 10 or 100 to limit movement */
#define RESIDENT_CHANCE_OF_ERRAND 500
#define VISITOR_CHANCE_OF_VISIT 1000

string name_list[20] = {"John","Mark","Paul","Lisa",
"Anne","Tony","Mike","Ryan","Noah","Ella","Leah","Zane",
"Jake","Emma","Mary","Jack","Alex","Luke","Rose","Sean"};

class Elevator;
class Floor;
class Building;
class Person;
class Resident;
class Visitor;

Building *leftmost_building;


class Elevator {
    private :
    int currentFloor;
    char *direction;
    int building_nFloors;
    int isAvailable;
    int destination;
    color c;
    Person *passenger;

    void update_movement();

    public :

    int x;

    int get_floor();
    char *get_direction();
    int get_destination();
    int is_available();
    color get_color();
    void set_destination(int floor);
    void set_passenger(Person *person);
    void free();
    void update();

    Elevator (int floor, char *arrow, int nFloors);
};

class Floor {
    private:
    int level;
    Building *building;

    public:
    int get_floor();
    Building *get_building();
    void call_elevator(Person *person);

    Floor (int num_floor, Building *building_ref);
};

class Building {
    private:
    Screen *s;
    int nFloors;
    int nColumns;
    int nElevators;

    Elevator* elevators[MAX_ELEVATOR_NUM];
    Floor* floors[MAX_FLOORS];

    void show_building();
    void show_elevators();

    public:

    Building* previousBuilding;
    Building* nextBuilding;

    int extraLeftOffset;
    int address;

    /* Return the closest available elevator */
    Elevator *get_nearest_available_elevator(int requested_floor);
    Floor *get_floor_from_int(int i);
    Screen *get_screen();
    int get_num_columns();
    void update();

    void dump();
    void elevators_dump();
    void set_previous_building(Building *building);
    void set_next_building(Building *building);

    Building (int num_floors, int num_elevators, 
        int building_address, Screen *screen);
};

class Person {

    friend class Resident;
    friend class Visitor;

    private :

    int final_destination_building;
    int final_destination_floor;
    int destination;
    int inElevator;
    int inBuilding;
    int isBetweenBuildings;
    int idle;

    int x;

    Elevator *elevator;
    Floor *floor;
    Building *building;
    Screen *s;

    public :

    int get_destination();
    int is_in_elevator();
    int is_at_destination();
    int is_idle();
    Elevator *get_elevator();
    int get_floor();
    Building *get_building();
    void set_elevator(Elevator *allocated_elevator);
    void update_movement();
    void show();
    void proc_final_destination(int destination_building, int destination_floor);

    Person (Building *current_building, int current_floor);
    Person () {}
};

class Resident : public Person {
    private :

    int home_building;
    int home_floor;

    int onErrand;
    int errand_tick_counter;

    void next_step();

    public :

    void go_home();
    void show();
    void proc_errand(int destination_building, int destination_floor);
    void update();
    int is_at_home();

    void dump();

    string name;

    Building *get_building() {return building;}

    Resident(Building *h_building, int h_floor, string identifier);
};

class Visitor : public Person {
    private :

    int exit_protocol;
    int visit_tick_counter;
    int onVisit;

    void leave();
    void next_step();

    public :

    void update();
    void proc_visit(int destination_building, int destination_floor);
    void show();
    void dump();

    Visitor(int destination_building, int destination_floor);
};