/* Header file for simulation state class */

// Include statements
#include <list>

// Structure to hold onto simulation properties and physical constants
struct SimParams
{
    // Constructors
    SimParams();
    SimParams(float lengthScale, float viscosity, float diffusion);
    SimParams(float lengthScale, float viscosity, float diffusion, 
                float gravity, float airDensity, float massRatio);
    SimParams(float lengthScale, float viscosity, float diffusion, 
                float gravity, float airDensity, float massRatio, 
                float airTemp, float diffTemp);
    SimParams(float lengthScale, float viscosity, float diffusion, 
                float gravity, float airDensity, float massRatio, 
                float airTemp, float diffTemp,
                float densDecay, float tempDecay);

    // Options
    bool advancedCoefficients;
    bool gravityOn;
    bool temperatureOn;
    int solverSteps;

    // Physical constants
    float lengthScale;
    float visc;
    float diff;
    float grav;
    float airDens;
    float massRatio;
    float airTemp;
    float diffTemp;
    float densDecay;
    float tempDecay;
};

// Structure to hold onto array pointers
struct SimFields
{
    // Constructors
    SimFields();
    SimFields(int size);

    // Current grid
    float * xVel;
    float * yVel;
    float * dens;
    float * temp;

    // Previous grid
    float * xVel_prev;
    float * yVel_prev;
    float * dens_prev;
    float * temp_prev;

    // Source grid
    float * xVel_source;
    float * yVel_source;
    float * dens_source;
    float * temp_source;
};

// Class which defines and contains important simulation methods
class SimState
{
    public:

        // Constructors
        SimState();
        SimState(int N);
        SimState(int N, SimParams params);

        // Public methods
        void SetSources(float * density, float * xVelocity, float * yVelocity, float * temperature);
        void SimulationStep(float timeStep);

        // Array accessors
        float * GetDensity();
        float * GetXVelocity();
        float * GetYVelocity();
        float * GetTemperature();

        // Modified fields
        static float MixedDensity(int ind, SimParams params, SimFields fields);
        static float MixedDensityAtAirTemp(int ind, SimParams params, SimFields fields);
        static float MixedTemperature(int ind, SimParams params, SimFields fields);
        static float AdjustedMassDiffusivity(int ind, SimParams params, SimFields fields);
        static float AdjustedViscosity(int ind, SimParams params, SimFields fields);
        static float AdjustedThermalDiffusivity(int ind, SimParams params, SimFields fields);

        // Grid size accessors
        int GetN();
        int GetSize();

        // Parameter struct
        SimParams params;

        // Array struct
        SimFields fields;

    private:

        // Grid size
        int N;
        int size;

        // Internal Methods
        void ZeroArrays();
        void SetSource(float *, float *);
        void SetConstantSource(float *, float);
        void AddSource(float *, float *, float);
        void AddConstantSource(float *, float, float);

        void DiffuseImproved(int b, float * x, float * x0, float (*diff)(int, SimParams, SimFields), float dt);
        void Dissipate(float *, float, float, float);
        void Advect(int, float *, float *, float *, float *, float);
        void Convect(float *, float);

        void SetBoundary(int, float *);
        void HodgeProjection(float *, float *, float *, float *);

        void DensityStep(float);
        void VelocityStep(float);
        void TemperatureStep(float);
};

// Structure which contains a density, velocity, or heat source for simulator
class SimSource
{
    public:

        // Constructor
        SimSource(SimState*);

        // Enumerable type designators
        enum Shape { square, circle, diamond };
        enum Type  { gas, wind, heat, energy };

        // SimState object
        SimState* simState;

        // Public methods
        void CreateGasSource(   Shape shape, float flowRate, float temp, float xCenter, float yCenter, float radius);
        void CreateWindSource(  Shape shape, float angle, float speed, float xCenter, float yCenter, float radius);
        void CreateHeatSource(  Shape shape, float temp, float xCenter, float yCenter, float radius);
        void CreateEnergySource(Shape shape, float flux, float referenceTemp, float xCenter, float yCenter, float radius);

        // Update sim object
        void UpdateSources();

    protected:

        // Saved data from SimState
        int N;
        int size;
        float lengthScale;

        // Pointers to source grids
        float * xVel;
        float * yVel;
        float * dens;
        float * temp;

        // Sub-class for single source
        class Source
        {
            public:

                // Enumerable type designators
                Shape shape;
                Type type;

                // Size of source
                float radius;

                // Public methods
                void SetActive(bool isActive);

                // Active flag
                bool isActive = true;

                // Arrays of sources
                std::list<int> indices;
                float xVel;
                float yVel;
                float dens;
                float temp;

                // Index calculation method
                void SetIndices(int N, Shape shape, float xCenter, float yCenter, float radius);
        };

        class GasSource: public Source { 
            public:
                GasSource(int N, float lengthScale, Shape shape, float flowRate, float temp, float xCenter, float yCenter, float radius);
                float flowRate, temp; };

        class WindSource: public Source { float speed, direction; };

        class HeatSource: public Source { float temp; };

        class EnergySource: public Source { float flux, referenceTemp; };

        // List of sources
        std::list<SimSource::Source*> sources;
};

// Method to display simulation state in terminal
void DisplayGrid(SimState stateInput, float minimum);