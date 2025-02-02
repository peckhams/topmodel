#include "../include/topmodel.h" 
#include "../include/bmi.h" 
#include "../include/bmi_topmodel.h"

/* BMI Adaption: Max i/o file name length changed from 30 to 256 */
#define MAX_FILENAME_LENGTH 256
#define OUTPUT_VAR_NAME_COUNT 14
#define INPUT_VAR_NAME_COUNT 2
#define STATE_VAR_NAME_COUNT 62   // must match var_info array size

//----------------------------------------------
// Put variable info into a struct to simplify
// BMI implementation and avoid errors.
//----------------------------------------------
// Should we add "/0" after each string here?
// Everything works without it.
//---------------------------------------------- 
Variable var_info[] = {
    //-------------------------------------
    // File pointers.  For reference only
    //-------------------------------------
	{ 0,  "control_fptr", "FILE", 1 },
	{ 1,  "input_fptr",   "FILE", 1 },
	{ 2,  "subcat_fptr",  "FILE", 1 },
	{ 3,  "params_fptr",  "FILE", 1 },
	{ 4,  "output_fptr",  "FILE", 1 },
	{ 5,  "out_hyd_fptr", "FILE", 1 },
    //----------------------------------------------
    // String vars.  Will replace 1 w/ title_size.
    //----------------------------------------------
	{ 6,  "title",  "string", 1 },
	{ 7,  "subcat", "string", 1 },   
    //-----------------------
    // Variable definitions
    //-----------------------
	{ 8,  "dt",                 "double", 1 },
	{ 9,  "nstep",              "int",    1 },
	{ 10, "yes_print_output",   "int",    1 },
	{ 11, "imap",               "int",    1 },
	{ 12, "num_channels",       "int",    1 },
	{ 13, "num_topodex_values", "int",    1 },
	{ 14, "infex",              "int",    1 },			  
    //-------------------------------------
    // Model parameters and input scalars
    //-------------------------------------
	{ 15,  "szm",        "double", 1 },
	{ 16,  "t0",         "double", 1 },
	{ 17,  "td",         "double", 1 },
	{ 18,  "srmax",      "double", 1 },
	{ 19,  "Q0",         "double", 1 },
	{ 20,  "sr0",        "double", 1 },
	{ 21,  "xk0",        "double", 1 },
	{ 22,  "hf",         "double", 1 },
	{ 23,  "dth",        "double", 1 },
	{ 24,  "area",       "double", 1 },
	{ 25,  "num_delay",  "int", 1 },
	{ 26,  "num_time_delay_hist_ords",  "int", 1 },
	{ 27,  "szq",              "double", 1 },
	{ 28,  "tl",               "double", 1 },
	{ 29,  "max_contrib_area", "double", 1 },
	{ 30,  "bal",              "double", 1 },
	{ 31,  "sbar",             "double", 1 },
    //------------------------------------------------
    // Pointers to dynamically dimensioned 1D arrays
    // Will replace size of 1 with size in comment
    // once those vars are defined.
    //------------------------------------------------
    // A trailing asterisk indicates that the var
    // is actually a pointer to the given type.
    //------------------------------------------------ 
	{ 32,  "Q",                        "double*", 1 },  // n_steps
	{ 33,  "Qobs",                     "double*", 1 },  // n_steps
	{ 34,  "rain",                     "double*", 1 },  // n_steps
	{ 35,  "pe",                       "double*", 1 },  // n_steps
	{ 36,  "contrib_area",             "double*", 1 },  // n_steps
	{ 37,  "stor_unsat_zone",          "double*", 1 },  // max_atb_incs
	{ 38,  "deficit_root_zone",        "double*", 1 },  // max_atb_incs
	{ 39,  "deficit_local",            "double*", 1 },  // max_atb_incs
	{ 40,  "time_delay_histogram",     "double*", 1 },  // max_td_ords
	{ 41,  "dist_area_lnaotb",         "double*", 1 },  // max_n_incs
	{ 42,  "lnaotb",                   "double*", 1 },  // max_n_incs
	{ 43,  "cum_dist_area_with_dist",  "double*", 1 },  // max_n_subcats
	{ 44,  "dist_from_outlet",         "double*", 1 },  // max_n_subcats	
    //---------------------- 
    // Other internal vars
    //----------------------
	{ 45,  "num_sub_catchments",       "int", 1 },
	{ 46,  "max_atb_increments",       "int", 1 },
	{ 47,  "max_num_subcatchments",    "int", 1 },
	{ 48,  "max_time_delay_ordinates", "int", 1 },
	{ 49,  "Qout",                     "double", 1 }, // Output var  
	{ 50,  "current_time_step",        "int", 1 },    // BMI var
    //-----------------
    // State var sums
    //-----------------
	{ 51,  "sump",  "double", 1 },
	{ 52,  "sumae", "double", 1 },
	{ 53,  "sumq",  "double", 1 },
	{ 54,  "sumrz", "double", 1 },
	{ 55,  "sumuz", "double", 1 },
    //----------------------    
    // External/forcing vars
    //----------------------
    { 56, "quz",         "double", 1 },
    { 57, "qb",          "double", 1 },
    { 58, "q0f",         "double", 1 },
    { 59, "p",           "double", 1 },
    { 60, "ep",          "double", 1 },
    { 61, "stand_alone", "int",    1 }
    // { 62, "obs_values",      "double", 1 },    
    // { 63, "double_arr_test", "double", 3 }             
};
  
static const char *output_var_names[OUTPUT_VAR_NAME_COUNT] = {
        "Qout",
        "atmosphere_water__domain_time_integral_of_rainfall_volume_flux",   //p
        "land_surface_water__potential_evaporation_volume_flux",            //ep
        "land_surface_water__runoff_mass_flux",                             //Q[it]
        "soil_water_root-zone_unsat-zone_top__recharge_volume_flux",        //qz
        "land_surface_water__baseflow_volume_flux",                         //qb
        "soil_water__domain_volume_deficit",                                //sbar
        "land_surface_water__domain_time_integral_of_overland_flow_volume_flux",    //qof
        "land_surface_water__domain_time_integral_of_precipitation_volume_flux",    //sump
        "land_surface_water__domain_time_integral_of_evaporation_volume_flux",      //sumae
        "land_surface_water__domain_time_integral_of_runoff_volume_flux",           //sumq
        "soil_water__domain_root-zone_volume_deficit",  //sumrz
        "soil_water__domain_unsaturated-zone_volume",   //sumuz
        "land_surface_water__water_balance_volume"      //bal
};

static const char *output_var_types[OUTPUT_VAR_NAME_COUNT] = {
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double"
};

static const int output_var_item_count[OUTPUT_VAR_NAME_COUNT] = {
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1
};

static const char *output_var_units[OUTPUT_VAR_NAME_COUNT] = {
        "m h-1",
        "m h-1",
        "m h-1",
        "m h-1",
        "m h-1",
        "m h-1",
        "m",
        "m h-1",
        "m",
        "m",
        "m",
        "m",
        "m",
        "m"
};

static const int output_var_grids[OUTPUT_VAR_NAME_COUNT] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
};

static const char *output_var_locations[OUTPUT_VAR_NAME_COUNT] = {
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node"
};

static const char *input_var_names[INPUT_VAR_NAME_COUNT] = {
        "atmosphere_water__liquid_equivalent_precipitation_rate",
        "water_potential_evaporation_flux"
};

static const char *input_var_types[INPUT_VAR_NAME_COUNT] = {
        "double",
        "double"
};

static const char *input_var_units[INPUT_VAR_NAME_COUNT] = {
        "kg m-2",
        "m s-1"
};

static const int input_var_item_count[INPUT_VAR_NAME_COUNT] = {
        1,
        1
};

static const char input_var_grids[INPUT_VAR_NAME_COUNT] = {
        0,
        0
};

static const char *input_var_locations[INPUT_VAR_NAME_COUNT] = {
        "node",
        "node"
};

int read_init_config(const char* config_file, topmodel_model* model) {
    
    // Open the primary config file
    /* BMI Adaption: No longer needs to be named "topmod.run" */
    if((model->control_fptr=fopen(config_file,"r"))==NULL){
        printf("Can't open control file named %s\n",config_file);      
        exit(-9);
    }

    /* BMI Adaption: Include stand_alone (bool) in config

    Structure of config_file as follows: 
    stand_alone
    title
    path/to/inputs.dat
    path/to/subcat.dat
    path/to/params.dat
    path/to/topmod.out
    path/to/hyd.out */

    //Read the stand_alone T/F
    //note: newline is needed here!
    fscanf(model->control_fptr,"%d\n",&model->stand_alone);

    //Read the title line, up to 255 characters, of the the file
    fgets(model->title,256,model->control_fptr);
    
    //Read a string, breaks on whitespace (or newline)
    //These must be done IN ORDER
    char input_fname[MAX_FILENAME_LENGTH];
    //It might be worth always scanning this line, but only opening the file if not STAND_ALONE
    fscanf(model->control_fptr,"%s",input_fname);
    
    //If stand_alone TRUE, read inputs from input file
    if (model->stand_alone == TRUE){
        if((model->input_fptr=fopen(input_fname,"r"))==NULL){
            printf("Can't open input file named %s\n",input_fname);
            exit(-9);
        }
    };

    char subcat_fname[MAX_FILENAME_LENGTH],params_fname[MAX_FILENAME_LENGTH];
    fscanf(model->control_fptr,"%s",subcat_fname);
    fscanf(model->control_fptr,"%s",params_fname);

    char output_fname[MAX_FILENAME_LENGTH],out_hyd_fname[MAX_FILENAME_LENGTH];
    fscanf(model->control_fptr,"%s",output_fname);
    fscanf(model->control_fptr,"%s",out_hyd_fname);

    //Attempt to read the parsed input file names, bail if they cannot be read/created
    if((model->subcat_fptr=fopen(subcat_fname,"r"))==NULL){       
        printf("Can't open subcat file named %s\n",subcat_fname);
        exit(-9);
    }

    if((model->params_fptr=fopen(params_fname,"r"))==NULL){
        printf("Can't open params file named %s\n",params_fname);   
        exit(-9);
    }
    
    /* READ IN SUBCATCHMENT TOPOGRAPHIC DATA */
    // This is needed here to gather yes_print_output for possible outfile read-in
    // TODO: JG thought - 
    //      If framework will never want these out files, 
    //      just use model->stand_alone as control switch
    //      move line to init_config() with others
    fscanf(model->subcat_fptr,"%d %d %d",&model->num_sub_catchments,&model->imap,&model->yes_print_output);

    // Attempt to read the output file names only if printing to file
    if(model->yes_print_output == TRUE){
        if((model->output_fptr=fopen(output_fname,"w"))==NULL){           
            printf("Can't open output file named %s\n",output_fname);
            exit(-9);
        }

        if((model->out_hyd_fptr=fopen(out_hyd_fname,"w"))==NULL){          
            printf("Can't open output file named %s\n",out_hyd_fname);
            exit(-9);
        }

        fprintf(model->output_fptr,"%s\n",model->title);

    }    
 
#if TOPMODEL_DEBUG >= 1    
    printf("TOPMODEL Version: TMOD95.02\n");
    printf("This run: %s\n",model->title);
#endif

    fclose(model->control_fptr);
    // Note all individual input files closed in init_config(),
    // which calls this function read_init_config()
    // Output files (if opened) closed in finalize()

    return BMI_SUCCESS;

}

int init_config(const char* config_file, topmodel_model* model)
{
    read_init_config(config_file,model);
    
    if (model->stand_alone == TRUE){
        /* READ IN nstep, DT and RAINFALL, PE, QOBS INPUTS */
        inputs(model->input_fptr, &model->nstep, &model->dt, &model->rain, &model->pe, 
            &model->Qobs, &model->Q, &model->contrib_area);
        fclose(model->input_fptr);
    }
    else {
        
        /* Set nstep and dt*/
        model->nstep = 1;
        model->dt = 1;

        /* allocate memory for "arrays" */
        d_alloc(&model->rain,model->nstep);
        d_alloc(&model->pe,model->nstep);
        d_alloc(&model->Qobs,model->nstep);   //TODO: Consider removing this all together when framework
        d_alloc(&model->Q,model->nstep);
        d_alloc(&model->contrib_area,model->nstep);

        (model->rain)[1]=0.0;
        (model->pe)[1]=0.0;
        (model->Qobs)[1]=0.0;
        (model->Q)[1]=0.0;
        (model->contrib_area)[1]=0.0;
    }

    // Set up maxes for subcat and params read-in functions
    model-> max_atb_increments=30;
    model-> max_num_subcatchments=10;
    model-> max_time_delay_ordinates=20;

    tread(model->subcat_fptr,model->output_fptr,model->subcat,&model->num_topodex_values,&model->num_channels,
        &model->area,&model->dist_area_lnaotb,&model->lnaotb,model->yes_print_output,
        &model->cum_dist_area_with_dist,&model->tl,&model->dist_from_outlet,
        model->max_num_subcatchments,model->max_atb_increments);
    fclose(model->subcat_fptr);

    init(model->params_fptr,model->output_fptr,model->subcat,model->num_channels,model->num_topodex_values,
        model->yes_print_output,model->area,&model->time_delay_histogram,model->cum_dist_area_with_dist,
        model->dt,&model->szm,&model->t0,model->tl,model->dist_from_outlet,&model->td, &model->srmax,&model->Q0,&model->sr0,&model->infex,&model->xk0,&model->hf,
        &model->dth,&model->stor_unsat_zone,&model->deficit_local,&model->deficit_root_zone,
        &model->szq,model->Q,&model->sbar,model->max_atb_increments,model->max_time_delay_ordinates,
        &model->bal,&model->num_time_delay_histo_ords,&model->num_delay);
    fclose(model->params_fptr);


    return BMI_SUCCESS;
}

// ***********************************************************
// ******************* BMI: TIME FUNCTIONS *******************
// ***********************************************************

static int Get_start_time (Bmi *self, double * time)
{
    *time = 0.0;
    return BMI_SUCCESS;
}

static int Get_end_time (Bmi *self, double * time)
{

    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;
    Get_start_time(self, time);
    
    // Standalone case gathers end_time via nstep dt
    if (topmodel->stand_alone == TRUE){
        *time += topmodel->nstep * topmodel->dt;
        return BMI_SUCCESS;
    
    // Otherwise, set to FLT_MAX macro via float.h
    // See https://bmi.readthedocs.io/en/latest/#get-end-time
    }
    else {
        *time += FLT_MAX;
        return BMI_SUCCESS;
    }
}

static int Get_time_step (Bmi *self, double * dt)
{
    *dt = ((topmodel_model *) self->data)->dt;
    return BMI_SUCCESS;
}

static int Get_time_units (Bmi *self, char * units)
{
    strncpy (units, "h", BMI_MAX_UNITS_NAME);
    return BMI_SUCCESS;
}

static int Get_current_time (Bmi *self, double * time)
{
    Get_start_time(self, time);
#if TOPMODEL_DEBUG > 1
    printf("Current model time step: '%d'\n", ((topmodel_model *) self->data)->current_time_step);
#endif
    *time += (((topmodel_model *) self->data)->current_time_step * 
              ((topmodel_model *) self->data)->dt);
    return BMI_SUCCESS;
}


// ***********************************************************
// *************** BMI: MODEL CONTROL FUNCTIONS **************
// ***********************************************************

static int Initialize (Bmi *self, const char *cfg_file)
{
    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;

    // Read and setup data from file
    init_config(cfg_file, topmodel);

    // Initialize model varables which are cumulatively defined
    topmodel->current_time_step=0;
    topmodel->sump = 0.0;
    topmodel->sumae = 0.0;
    topmodel->sumq = 0.0;

    return BMI_SUCCESS;
}

// This is not needed. Update_until does it already.
// static int Update_frac (void * self, double f)
// { /* Implement this: Update for a fraction of a time step */
//     return BMI_FAILURE;
// }

static int Update (Bmi *self)
{
    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;

    double current_time, end_time;
    self->get_current_time(self, &current_time);
    self->get_end_time(self, &end_time);
    if (current_time >= end_time) {
        return BMI_FAILURE;
    };

    topmodel->current_time_step += topmodel->dt;   

    topmod(topmodel->output_fptr,topmodel->nstep, topmodel->num_topodex_values,
        topmodel->yes_print_output,topmodel->infex, topmodel->dt, topmodel->szm,
        topmodel->stor_unsat_zone,topmodel->deficit_root_zone,
        topmodel->deficit_local, topmodel->pe, topmodel->rain,topmodel->xk0,topmodel->hf, 
        topmodel->dist_area_lnaotb, topmodel->tl, topmodel->lnaotb, topmodel->td,
        topmodel->srmax, topmodel->contrib_area, topmodel->szq, &topmodel->Qout, 
        topmodel->num_time_delay_histo_ords,topmodel->Q,
        topmodel->time_delay_histogram,topmodel->subcat,&topmodel->bal,
        &topmodel->sbar,topmodel->num_delay,topmodel->current_time_step, topmodel->stand_alone,
        &topmodel->sump,&topmodel->sumae,&topmodel->sumq,&topmodel->sumrz,&topmodel->sumuz,
        &topmodel->quz, &topmodel->qb, &topmodel->qof, &topmodel->p, &topmodel->ep );

    //--------------------------------------------------
    // This should be moved into the Finalize() method
    //--------------------------------------------------
    // results() 
    // 1. generates hydrograph out file (hyd.out)
    // 2. computes objective function stats
    //        - print to console
    //        - print to main out file (topmod.out)
    // Logic for each is handled indiv w.i. funct,
    // but wouldn't hurt to check conditions here as framework
    // will likely not even need to jump into results()
    if (topmodel->stand_alone == TRUE){
    results(topmodel->output_fptr, topmodel->out_hyd_fptr, topmodel->nstep, 
        topmodel->Qobs, topmodel->Q, 
        topmodel->current_time_step, topmodel->yes_print_output);
    }

    return BMI_SUCCESS;
}

static int Update_until (Bmi *self, double t)
{
    double dt;
    double now;

    if(self->get_time_step (self, &dt) == BMI_FAILURE)
        return BMI_FAILURE;

    if(self->get_current_time(self, &now) == BMI_FAILURE)
        return BMI_FAILURE;

    {
      int n;
      double frac;
      const double n_steps = (t - now) / dt;
      for (n=0; n<(int)n_steps; n++) {
        Update(self);
      }

      frac = n_steps - (int)n_steps;
      ((topmodel_model *)self->data)->dt = frac * dt;
      Update (self);
      ((topmodel_model *)self->data)->dt = dt;

    }

    return BMI_SUCCESS;
}

static int Finalize (Bmi *self)
{
  if (self){
    topmodel_model* model = (topmodel_model *)(self->data);

    //-----------------------------------------------------------
    // When running in stand-alone mode, the original "results"
    // method should be called here in the Finalize() method,
    // not in Update_until(). It could also be called when in
    // framework-controlled mode.
    //-----------------------------------------------------------
    //if (model->yes_print_output == TRUE || TOPMODEL_DEBUG >= 1){
    //results(model->output_fptr,model->out_hyd_fptr,model->nstep, 
    //    model->Qobs, model->Q, 
    //    model->current_time_step, model->yes_print_output);
    
    if (model->yes_print_output == TRUE || TOPMODEL_DEBUG >= 1){        
        
        water_balance(model->output_fptr, model->yes_print_output,
            model->subcat,&model->bal, &model->sbar, &model->sump, 
            &model->sumae, &model->sumq, &model->sumrz, &model->sumuz);
    }

    if( model->Q != NULL )
        free(model->Q);
    if( model->Qobs != NULL )
        free(model->Qobs);
    if( model->rain != NULL )
        free(model->rain);
    if( model->pe != NULL )
        free(model->pe);
    if( model->contrib_area != NULL )
        free(model->contrib_area);
    if( model->stor_unsat_zone != NULL )
        free(model->stor_unsat_zone);
    if( model->deficit_root_zone != NULL )
        free(model->deficit_root_zone);
    if( model->deficit_local != NULL )
        free(model->deficit_local);
    if( model->time_delay_histogram != NULL )
        free(model->time_delay_histogram);
    if( model->dist_area_lnaotb != NULL )
        free(model->dist_area_lnaotb);
    if( model->lnaotb != NULL )
        free(model->lnaotb);
    if( model->cum_dist_area_with_dist != NULL )
        free(model->cum_dist_area_with_dist);
    if( model->dist_from_outlet != NULL)
        free(model->dist_from_outlet);

    // Close output files only if opened in first place
    if(model->yes_print_output == TRUE){
        fclose(model->output_fptr);
        fclose(model->out_hyd_fptr);
    }
    
    free(self->data);
  }
    return BMI_SUCCESS;
}


// ***********************************************************
// *********** BMI: VARIABLE INFORMATION FUNCTIONS ***********
// ***********************************************************

static int Get_var_type (Bmi *self, const char *name, char * type)
{
    // Check to see if in output array first
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, output_var_names[i]) == 0) {
            strncpy(type, output_var_types[i], BMI_MAX_TYPE_NAME);
            return BMI_SUCCESS;
        }
    }
    // Then check to see if in input array
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            strncpy(type, input_var_types[i], BMI_MAX_TYPE_NAME);
            return BMI_SUCCESS;
        }
    }
    // If we get here, it means the variable name wasn't recognized
    type[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_grid(Bmi *self, const char *name, int *grid)
{

    // Check to see if in output array first
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, output_var_names[i]) == 0) {
            *grid = output_var_grids[i];
            return BMI_SUCCESS;
        }
    }
    // Then check to see if in input array
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            *grid = input_var_grids[i];
            return BMI_SUCCESS;
        }
    }
    // If we get here, it means the variable name wasn't recognized
    grid[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_itemsize (Bmi *self, const char *name, int * size)
{
    char type[BMI_MAX_TYPE_NAME];
    int type_result = Get_var_type(self, name, type);
    if (type_result != BMI_SUCCESS) {
        return BMI_FAILURE;
    }

    if (strcmp (type, "double") == 0) {
        *size = sizeof(double);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "float") == 0) {
        *size = sizeof(float);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "int") == 0) {
        *size = sizeof(int);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "short") == 0) {
        *size = sizeof(short);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "long") == 0) {
        *size = sizeof(long);
        return BMI_SUCCESS;
    }
    else {
        *size = 0;
        return BMI_FAILURE;
    }
}

static int Get_var_location (Bmi *self, const char *name, char * location)
{
    // Check to see if in output array first
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, output_var_names[i]) == 0) {
            strncpy(location, output_var_locations[i], BMI_MAX_LOCATION_NAME);
            return BMI_SUCCESS;
        }
    }
    // Then check to see if in input array
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            strncpy(location, input_var_locations[i], BMI_MAX_LOCATION_NAME);
            return BMI_SUCCESS;
        }
    }
    // If we get here, it means the variable name wasn't recognized
    location[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_units (Bmi *self, const char *name, char * units)
{
    // Check to see if in output array first
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, output_var_names[i]) == 0) {
            strncpy(units, output_var_units[i], BMI_MAX_UNITS_NAME);
            return BMI_SUCCESS;
        }
    }
    // Then check to see if in input array
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            strncpy(units, input_var_units[i], BMI_MAX_UNITS_NAME);
            return BMI_SUCCESS;
        }
    }
    // If we get here, it means the variable name wasn't recognized
    units[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_nbytes (Bmi *self, const char *name, int * nbytes)
{
    int item_size;
    int item_size_result = Get_var_itemsize(self, name, &item_size);
    if (item_size_result != BMI_SUCCESS) {
        return BMI_FAILURE;
    }
    int item_count = -1;
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            item_count = input_var_item_count[i];
            break;
        }
    }
    if (item_count < 1) {
        for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
            if (strcmp(name, output_var_names[i]) == 0) {
                item_count = output_var_item_count[i];
                break;
            }
        }
    }
    if (item_count < 1)
        item_count = ((topmodel_model *) self->data)->nstep;

    *nbytes = item_size * item_count;
    return BMI_SUCCESS;
}


// ***********************************************************
// ********* BMI: VARIABLE GETTER & SETTER FUNCTIONS *********
// ***********************************************************

static int Get_value_ptr (Bmi *self, const char *name, void **dest)
{
    // Qout
    if (strcmp (name, "Qout") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> Qout;
        return BMI_SUCCESS;
    }
    // p
    if (strcmp (name, "atmosphere_water__domain_time_integral_of_rainfall_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> p;
        return BMI_SUCCESS;
    // ep    
    }
    if (strcmp (name, "land_surface_water__potential_evaporation_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> ep;
        return BMI_SUCCESS;
    }
    // Q[it]
    if (strcmp (name, "land_surface_water__runoff_mass_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> Q[1];
        return BMI_SUCCESS;
    }
    // quz
    if (strcmp (name, "soil_water_root-zone_unsat-zone_top__recharge_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> quz;
        return BMI_SUCCESS;
    }
    // qb
    if (strcmp (name, "land_surface_water__baseflow_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> qb;
        return BMI_SUCCESS;
    }
    // sbar
    if (strcmp (name, "soil_water__domain_volume_deficit") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sbar;
        return BMI_SUCCESS;
    }
    // qof
    if (strcmp (name, "land_surface_water__domain_time_integral_of_overland_flow_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> qof;
        return BMI_SUCCESS;
    }
    // sump
    if (strcmp (name, "land_surface_water__domain_time_integral_of_precipitation_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sump;
        return BMI_SUCCESS;
    }
    // sumae
    if (strcmp (name, "land_surface_water__domain_time_integral_of_evaporation_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sumae;
        return BMI_SUCCESS;
    }// sumq
    if (strcmp (name, "land_surface_water__domain_time_integral_of_runoff_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sumq;
        return BMI_SUCCESS;
    }
    // sumrz
    if (strcmp (name, "soil_water__domain_root-zone_volume_deficit") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sumrz;
        return BMI_SUCCESS;
    }
    // sumuz
    if (strcmp (name, "soil_water__domain_unsaturated-zone_volume") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sumuz;
        return BMI_SUCCESS;
    }
    // bal
    if (strcmp (name, "land_surface_water__water_balance_volume") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> bal;
        return BMI_SUCCESS;
    }

    // STANDALONE Note: 
    //      When TRUE/1 there are no bmi inputs being passed
    //      defs here speak to "scalar"  
    //      TODO: add logic to only apply these defs for framework runs 
    if (strcmp (name, "water_potential_evaporation_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> pe[1];
        //*dest = (void*)(topmodel->pe + topmodel->current_time_step);
        
        return BMI_SUCCESS;
    }

    if (strcmp (name, "atmosphere_water__liquid_equivalent_precipitation_rate") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel->rain[1];
        return BMI_SUCCESS;
    }

    return BMI_FAILURE;
}

static int Get_value_at_indices (Bmi *self, const char *name, void *dest, int * inds, int len)
{
    void *src = NULL;
    int itemsize = 0;

    if (self->get_value_ptr(self, name, &src) == BMI_FAILURE)
        return BMI_FAILURE;

    if (self->get_var_itemsize(self, name, &itemsize) == BMI_FAILURE)
        return BMI_FAILURE;

    { /* Copy the data */
        size_t i;
        size_t offset;
        char * ptr;
        for (i=0, ptr=(char*)dest; i<len; i++, ptr+=itemsize) {
            offset = inds[i] * itemsize;
            memcpy (ptr, (char*)src + offset, itemsize);
        }
    }

    return BMI_SUCCESS;
}

static int Get_value(Bmi * self, const char * name, void *dest)
{
    void *src = NULL;
    int nbytes = 0;

    if (self->get_value_ptr (self, name, &src) == BMI_FAILURE)
        return BMI_FAILURE;

    if (self->get_var_nbytes (self, name, &nbytes) == BMI_FAILURE)
        return BMI_FAILURE;

    memcpy(dest, src, nbytes);

    return BMI_SUCCESS;
}

static int Set_value (Bmi *self, const char *name, void *array)
{
    void * dest = NULL;
    int nbytes = 0;

    if (self->get_value_ptr(self, name, &dest) == BMI_FAILURE)
        return BMI_FAILURE;

    if (self->get_var_nbytes(self, name, &nbytes) == BMI_FAILURE)
        return BMI_FAILURE;

    memcpy (dest, array, nbytes);

    return BMI_SUCCESS;
}

static int Set_value_at_indices (Bmi *self, const char *name, int * inds, int len, void *src)
{
    void * to = NULL;
    int itemsize = 0;

    if (self->get_value_ptr (self, name, &to) == BMI_FAILURE)
        return BMI_FAILURE;

    if (self->get_var_itemsize(self, name, &itemsize) == BMI_FAILURE)
        return BMI_FAILURE;

    { /* Copy the data */
        size_t i;
        size_t offset;
        char * ptr;
        for (i=0, ptr=(char*)src; i<len; i++, ptr+=itemsize) {
            offset = inds[i] * itemsize;
            memcpy ((char*)to + offset, ptr, itemsize);
        }
    }
    return BMI_SUCCESS;
}

// ***********************************************************
// *****  NEW BMI: STATE VAR GETTER & SETTER FUNCTIONS   *****
// *****  Proposed extensions to support serialization.  *****
// ***********************************************************
static int Get_state_var_count (Bmi *self, int * count)
{
    if (!self){
        return BMI_FAILURE;   
    }

    *count = STATE_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}
//--------------------------------------------------------------------------
static int Get_state_var_names (Bmi *self, char ** names)
{
    //---------------------------------------------------
    // Note: This pulls information from the var_info
    // structure defined at the top, which helps to
    // prevent implementation errors.
    //---------------------------------------------------
    // This is used for model state serialization, and
    // returns a string array of all state variable
    // names, in same order as defined in state struct.
    // These names can simply be internal vs. standard
    // names because they are not used for coupling.
    //---------------------------------------------------
    if (!self){
        return BMI_FAILURE;   
    }

    int n_state_vars = STATE_VAR_NAME_COUNT;
    int MAX_NAME_LEN = 512;
    //int MAX_NAME_LEN = BMI_MAX_VAR_NAME;

    for (int i = 0; i < n_state_vars; i++) {
        strncpy(names[i], var_info[i].name, MAX_NAME_LEN);

        //--------------------------------  
        // Option to print all the names
        //--------------------------------
        // if (i==0) printf(" State variable names:");
        // printf(" var name[%d] = %s\n", i, names[i]);
    }
        
    return BMI_SUCCESS;
}

//--------------------------------------------------------------------------
static int Get_state_var_types (Bmi *self, char ** types)
{
    //---------------------------------------------------
    // Note: This pulls information from the var_info
    // structure defined at the top, which helps to 
    // prevent implementation errors.   
    //---------------------------------------------------
    // This is used for model state serialization, and
    // returns a string array of all state variable
    // types, in same order as defined in state struct.
    // Later, bmi.get_var_type() may be extended to
    // get more than input & output variable types.
    //---------------------------------------------------
    if (!self){
        return BMI_FAILURE;   
    }

    int n_state_vars = STATE_VAR_NAME_COUNT;    
    int MAX_NAME_LEN = 512;
    //int MAX_NAME_LEN = BMI_MAX_VAR_NAME;
 
    for (int i = 0; i < n_state_vars; i++) {
        strncpy(types[i], var_info[i].type, MAX_NAME_LEN);
        
        //--------------------------------  
        // Option to print all the types
        //--------------------------------
        // if (i==0) printf(" State var_types:");
        // printf(" var type[%d] = %s\n", i, types[i]);
    }
        
    return BMI_SUCCESS;
}

//--------------------------------------------------------------------------
static int Get_state_var_sizes (Bmi *self, unsigned int size_list[])
{
    //---------------------------------------------------
    // Note: This pulls information from the var_info
    // structure defined at the top, which helps to 
    // prevent implementation errors.   
    //---------------------------------------------------
    // This is used for model state serialization, and
    // returns a string array of all state variable
    // sizes, in same order as defined in state struct.
    // Size is number of array elements (not bytes).
    // Just number of elements, even for n-dim arrays.
    //---------------------------------------------------
    if (!self){
        return BMI_FAILURE;   
    }

    topmodel_model *state;
    state = (topmodel_model*) self->data;
    int n_state_vars = STATE_VAR_NAME_COUNT;

    //------------------------------------------------     
    // NOTE:  max_num_increments is just a local var
    //        name for max_atb_increments.
    // NOTE:  max_n_incs = max_atb_incs + 1
    //-----------------------------------------------------
    // NOTE:  TOPMODEL uses d_alloc() to allocate memory
    //        for arrays, but adds 1 to the array size
    //        and then loop counts start at 1 not 0.
    //        Also, d_alloc() is often called with a +1.
    //        So we need to add 1 to all of these.
    //-----------------------------------------------------
    // What about:  num_channels, num_topodex_values, num_delay,
    //   num_time_delay_histo_ords ??
    // num_topodex_values = max_atb_increments ??  ##########
    //------------------------------------------------------------ 
    unsigned int title_size = 257;   // see topmodel.h; char array
    unsigned int n_steps       = state->nstep+1;
    unsigned int max_n_subcats = state->max_num_subcatchments+1;
    unsigned int max_n_incs    = state->max_atb_increments+1;
    unsigned int max_atb_incs  = state->max_atb_increments+1;
    unsigned int max_td_ords   = state->max_time_delay_ordinates+1;
  
    // For testing
    //printf("In get_state_var_sizes():\n");
    //printf("  n_steps       = %d\n", n_steps);
    //printf("  max_n_subcats = %d\n", max_n_subcats);
    //printf("  max_atb_incs  = %d\n", max_atb_incs);
    //printf("  max_td_ords   = %d\n", max_td_ords);
    //printf("");

    //-------------------------------------------------
    // Overwrite the sizes that are not 1 (now known)
    //-------------------------------------------------
    var_info[6].size  = title_size;     // title
    var_info[7].size  = title_size;     // subcat
    //---------------------------------------------    
    var_info[32].size = n_steps;        // Q
    var_info[33].size = n_steps;        // Qobs
    var_info[34].size = n_steps;        // rain
    var_info[35].size = n_steps;        // pe
    var_info[36].size = n_steps;        // contrib_area
    var_info[37].size = max_atb_incs;   // stor_unsat_zone
    var_info[38].size = max_atb_incs;   // deficit_root_zone
    var_info[39].size = max_atb_incs;   // deficit_local
    var_info[40].size = max_td_ords;    // time_delay_histogram
    var_info[41].size = max_n_incs;     // dist_area_lnaotb
    var_info[42].size = max_n_incs;     // lnaotb
    var_info[43].size = max_n_subcats;  // cum_dist_area_with_dist
    var_info[44].size = max_n_subcats;  // dist_from_outlet
       
    for (int i = 0; i < n_state_vars; i++) {
        size_list[i] = var_info[i].size;
    }
  
    return BMI_SUCCESS;
}

//-----------------------------------------------------------------------
static int Get_state_var_ptrs (Bmi *self, void *ptr_list[])
{
    //----------------------------------------------
    // Return array of pointers to state variables
    // in same order as defined in state struct.
    //----------------------------------------------
    if (!self){
        return BMI_FAILURE;   
    }

    topmodel_model *state;
    state = (topmodel_model*) self->data;  // typecast self->data

    //--------------------------------------
    // Create a test array: dbl_array_test
    //--------------------------------------
    /* double dbl_test_data[3] = {10.0, 20.0, 30.0};
    for (int j = 0; j < 3; j++) {
        state->dbl_arr_test[j] = dbl_test_data[j];
    } */   

    ptr_list[0]  = state->control_fptr;
    ptr_list[1]  = state->input_fptr;    
    ptr_list[2]  = state->subcat_fptr;
    ptr_list[3]  = state->params_fptr;
    ptr_list[4]  = state->output_fptr;
    ptr_list[5]  = state->out_hyd_fptr;    
    ptr_list[6]  = &(state->title);
    ptr_list[7]  = &(state->subcat);
    ptr_list[8]  = &(state->dt);
    ptr_list[9]  = &(state->nstep);
    ptr_list[10] = &(state->yes_print_output);
    ptr_list[11] = &(state->imap);
    ptr_list[12] = &(state->num_channels);    
    ptr_list[13] = &(state->num_topodex_values);
    ptr_list[14] = &(state->infex);
    ptr_list[15] = &(state->szm);
    ptr_list[16] = &(state->t0);
    ptr_list[17] = &(state->td);
    ptr_list[18] = &(state->srmax);
    ptr_list[19] = &(state->Q0);    
    ptr_list[20] = &(state->sr0);
    ptr_list[21] = &(state->xk0);
    ptr_list[22] = &(state->hf);
    ptr_list[23] = &(state->dth);
    ptr_list[24] = &(state->area);
    ptr_list[25] = &(state->num_delay);
    ptr_list[26] = &(state->num_time_delay_histo_ords); 
    ptr_list[27] = &(state->szq);
    ptr_list[28] = &(state->tl);
    ptr_list[29] = &(state->max_contrib_area);
    ptr_list[30] = &(state->bal);
    ptr_list[31] = &(state->sbar);
    //----------------------------------------------------
    // These vars ARE pointers to different-sized arrays
    // Do not append "&".
    //----------------------------------------------------         
    ptr_list[32] = state->Q;
    ptr_list[33] = state->Qobs;
    ptr_list[34] = state->rain;
    ptr_list[35] = state->pe;
    ptr_list[36] = state->contrib_area;
    ptr_list[37] = state->stor_unsat_zone;
    ptr_list[38] = state->deficit_root_zone; 
    ptr_list[39] = state->deficit_local;
    ptr_list[40] = state->time_delay_histogram;
    ptr_list[41] = state->dist_area_lnaotb;
    ptr_list[42] = state->lnaotb;
    ptr_list[43] = state->cum_dist_area_with_dist; 
    ptr_list[44] = state->dist_from_outlet;
    //----------------------------------------------------  
    ptr_list[45] = &(state->num_sub_catchments);
    ptr_list[46] = &(state->max_atb_increments);
    ptr_list[47] = &(state->max_num_subcatchments);
    ptr_list[48] = &(state->max_time_delay_ordinates);
    ptr_list[49] = &(state->Qout);
    ptr_list[50] = &(state->current_time_step); 
    ptr_list[51] = &(state->sump);
    ptr_list[52] = &(state->sumae);
    ptr_list[53] = &(state->sumq);
    ptr_list[54] = &(state->sumrz);
    ptr_list[55] = &(state->sumuz);
    ptr_list[56] = &(state->quz);
    ptr_list[57] = &(state->qb);
    ptr_list[58] = &(state->qof);
    ptr_list[59] = &(state->p);
    ptr_list[60] = &(state->ep);
    ptr_list[61] = &(state->stand_alone);
    //ptr_list[62] = &(state->dbl_arr_test);  // this is correct
        
    return BMI_SUCCESS;
}
 
//-----------------------------------------------------------------------
static int Set_state_var (Bmi *self, void *src, int index)
{
    //----------------------------------------------------
    // Set the value (or values) for a state variable
    // using its position index within the state struct.
    //----------------------------------------------------
    if (!self){
        return BMI_FAILURE;
    }

    int n_state_vars, i;
    self->get_state_var_count(self, &n_state_vars);  
    unsigned int sizes[ n_state_vars ];
    self->get_state_var_sizes(self, sizes);
    unsigned int size = sizes[ index ];

    topmodel_model *state;
    state = (topmodel_model*) self->data;

    //---------------------------------------------
    // Set value of state variable given by index
    //--------------------------------------------------
    // The original file pointer is meaningless after
    // serialization, so use a null file pointer.
    // When necessary, a file pointer offset can be
    // stored, then file opened again and moved there.
    //--------------------------------------------------
    if (index == 0){
        state->control_fptr = fopen("/dev/null", "w"); }
    else if (index == 1){
        state->input_fptr   = fopen("/dev/null", "w"); }
    else if (index == 2){
        state->subcat_fptr  = fopen("/dev/null", "w"); }       
    else if (index == 3){
        state->params_fptr  = fopen("/dev/null", "w"); }        
    else if (index == 4){
        state->output_fptr  = fopen("/dev/null", "w"); }
    else if (index == 5){
        state->out_hyd_fptr = fopen("/dev/null", "w"); }
    //-----------------------------------------------------
    // This seems to work
    //---------------------
    else if (index == 6){
        memcpy(&state->title, src, size); }
    else if (index == 7){
        memcpy(&state->subcat, src, size); }
    //-----------------------------------------------------
    // This seems to work also
    //--------------------------
//     else if (index == 6){
//        for (i=0; i<size; i++) {
//            state->title[i] = *( (char *)src + i); } }
//     else if (index == 7){     
//        for (i=0; i<size; i++) {
//            state->subcat[i] = *( (char *)src + i); } }
    //-----------------------------------------------------     
    else if (index == 8){
        state->dt = *(double *)src; }  
    else if (index == 9){
        state->nstep = *(int *)src; }
    else if (index == 10){
        state->yes_print_output = *(int *)src; } 
    else if (index == 11){
        state->imap = *(int *)src; }
    else if (index == 12){
        state->num_channels = *(int *)src; } 
    else if (index == 13){
        state->num_topodex_values = *(int *)src; } 
    else if (index == 14){
        state->infex = *(int *)src; }
    else if (index == 15){
        state->szm = *(double *)src; }                                
    else if (index == 16){
        state->t0 = *(double *)src; } 
    else if (index == 17){
        state->td = *(double *)src; } 
    else if (index == 18){
        state->srmax = *(double *)src; }                 
    else if (index == 19){
        state->Q0 = *(double *)src; } 
    else if (index == 20){
        state->sr0 = *(double *)src; }         
    //-----------------------------------------------------
    else if (index == 21){
        state->xk0 = *(double *)src; }  
    else if (index == 22){
        state->hf = *(double *)src; }  
    else if (index == 23){
        state->dth = *(double *)src; }  
    else if (index == 24){
        state->area = *(double *)src; }                  
    else if (index == 25){
        state->num_delay = *(int *)src; }  
    else if (index == 26){
        state->num_time_delay_histo_ords = *(int *)src; }  
    else if (index == 27){
        state->szq = *(double *)src; }  
    else if (index == 28){
        state->tl = *(double *)src; }                          
    else if (index == 29){
        state->max_contrib_area = *(double *)src; }  
    else if (index == 30){
        state->bal = *(double *)src; }          
    else if (index == 31){
        state->sbar = *(double *)src; }
    //-----------------------------------------------------
    // Indexes 32 to 44 are pointers to arrays
    //-----------------------------------------------------
    // NOTE! Typecast ptr first, then add offset,
    //       into array, then dereference the ptr
    //       CORRECT:    *( ((double *)src) + i)  ??
    //       INCORRECT:  *( (double *)(src + i))
    //       INCORRECT:  *( (double *)src + i)  ??
    //       INCORRECT:  *(double *)src + i 
    //---------------------------------------------
    // Note: state->Q is a pointer to an array
    //       We don't need to change that pointer,
    //       just the values in the array.
    //---------------------------------------------     
    else if (index == 32){
        for (i=0; i<size; i++) {
            state->Q[i] = *( ((double *)src) + i); } }
    else if (index == 33){
        for (i=0; i<size; i++) {
            state->Qobs[i] = *( ((double *)src) + i); } }
    else if (index == 34){
        for (i=0; i<size; i++) {
            state->rain[i] = *( ((double *)src) + i); } }
    else if (index == 35){
        for (i=0; i<size; i++) {
            state->pe[i] = *( ((double *)src) + i); } }                                    
    else if (index == 36){
        for (i=0; i<size; i++) {
            state->contrib_area[i] = *( ((double *)src) + i); } }   
    else if (index == 37){
        for (i=0; i<size; i++) {
            state->stor_unsat_zone[i] = *( ((double *)src) + i); } } 
    else if (index == 38){
        for (i=0; i<size; i++) {
            state->deficit_root_zone[i] = *( ((double *)src) + i); } } 
    else if (index == 39){
        for (i=0; i<size; i++) {
            state->deficit_local[i] = *( ((double *)src) + i); } }             
    else if (index == 40){
        for (i=0; i<size; i++) {
            state->time_delay_histogram[i] = *( ((double *)src) + i); } }                                     
    else if (index == 41){
        for (i=0; i<size; i++) {
            state->dist_area_lnaotb[i] = *( ((double *)src) + i); } } 
    else if (index == 42){
        for (i=0; i<size; i++) {
            state->lnaotb[i] = *( ((double *)src) + i); } } 
    else if (index == 43){
        for (i=0; i<size; i++) {
            state->cum_dist_area_with_dist[i] = *( ((double *)src) + i); } } 
    else if (index == 44){
        for (i=0; i<size; i++) {
            state->dist_from_outlet[i] = *( ((double *)src) + i); } }                               
    //-----------------------------------------------------
    else if (index == 45){
        state->num_sub_catchments = *(int *)src; }
    else if (index == 46){
        state->max_atb_increments = *(int *)src; }
    else if (index == 47){
        state->max_num_subcatchments = *(int *)src; }                
    else if (index == 48){
        state->max_time_delay_ordinates = *(int *)src; }
    else if (index == 49){
        state->Qout = *(double *)src; } 
    //-----------------------------------------------------                     
    else if (index == 50){
        state->current_time_step = *(int *)src; }
    else if (index == 51){
        state->sump = *(double *)src; }
    else if (index == 52){
        state->sumae = *(double *)src; }
    else if (index == 53){
        state->sumq = *(double *)src; }
    else if (index == 54){
        state->sumrz = *(double *)src; }
    else if (index == 55){
        state->sumuz = *(double *)src; }
    else if (index == 56){
        state->quz = *(double *)src; }
    else if (index == 57){
        state->qb = *(double *)src; }
    else if (index == 58){
        state->qof = *(double *)src; }
    else if (index == 59){
        state->p = *(double *)src; }
    else if (index == 60){
        state->ep = *(double *)src; }
    else if (index == 61){
        state->stand_alone = *(int *)src; }
    //-------------------------------------
    // This is a double array for testing
    //-------------------------------------
/*    else if (index == 62){
        for (i=0; i<size; i++) {
            state->dbl_arr_test[i] = *(double *)(src + i); } }*/

    return BMI_SUCCESS;
}
 

// ***********************************************************
// ************ BMI: MODEL INFORMATION FUNCTIONS *************
// ***********************************************************

static int Get_component_name (Bmi *self, char * name)
{
    strncpy (name, "TOPMODEL", BMI_MAX_COMPONENT_NAME);
    return BMI_SUCCESS;
}

static int Get_input_item_count (Bmi *self, int * count)
{
    *count = INPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}

static int Get_input_var_names (Bmi *self, char ** names)
{
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        strncpy (names[i], input_var_names[i], BMI_MAX_VAR_NAME);
    }
    return BMI_SUCCESS;
}

static int Get_output_item_count (Bmi *self, int * count)
{
    *count = OUTPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}

static int Get_output_var_names (Bmi *self, char ** names)
{
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        strncpy (names[i], output_var_names[i], BMI_MAX_VAR_NAME);
    }
    return BMI_SUCCESS;
}


// ***********************************************************
// **************** BMI: MODEL GRID FUNCTIONS ****************
// ***********************************************************

/* Grid information */
static int Get_grid_rank (Bmi *self, int grid, int * rank)
{
    if (grid == 0) {
        *rank = 1;
        return BMI_SUCCESS;
    }
    else {
        *rank = -1;
        return BMI_FAILURE;
    }
}

static int Get_grid_size(Bmi *self, int grid, int * size)
{
    if (grid == 0) {
        *size = 1;
        return BMI_SUCCESS;
    }
    else {
        *size = -1;
        return BMI_FAILURE;
    }
}

static int Get_grid_type (Bmi *self, int grid, char * type)
{
    int status = BMI_FAILURE;

    if (grid == 0) {
        strncpy(type, "scalar", BMI_MAX_TYPE_NAME);
        status = BMI_SUCCESS;
    }
    else {
        type[0] = '\0';
        status = BMI_FAILURE;
    }
    return status;
}

/* Uniform rectilinear (grid type) */
static int Get_grid_shape(Bmi *self, int grid, int *shape)
{
    return BMI_FAILURE;
}

static int Get_grid_spacing(Bmi *self, int grid, double *spacing)
{
    return BMI_FAILURE;
}

static int Get_grid_origin(Bmi *self, int grid, double *origin)
{
    return BMI_FAILURE;
}

/* Non-uniform rectilinear, curvilinear (grid type)*/
static int Get_grid_x(Bmi *self, int grid, double *x)
{
    return BMI_FAILURE;
}

static int Get_grid_y(Bmi *self, int grid, double *y)
{
    return BMI_FAILURE;
}

static int Get_grid_z(Bmi *self, int grid, double *z)
{
    return BMI_FAILURE;
}

/*Unstructured (grid type)*/
static int Get_grid_node_count(Bmi *self, int grid, int *count)
{
    return BMI_FAILURE;
}

static int Get_grid_edge_count(Bmi *self, int grid, int *count)
{
    return BMI_FAILURE;
}

static int Get_grid_face_count(Bmi *self, int grid, int *count)
{
    return BMI_FAILURE;
}

static int Get_grid_edge_nodes(Bmi *self, int grid, int *edge_nodes)
{
    return BMI_FAILURE;
}

static int Get_grid_face_edges(Bmi *self, int grid, int *face_edges)
{
    return BMI_FAILURE;
}

static int Get_grid_face_nodes(Bmi *self, int grid, int *face_nodes)
{
    return BMI_FAILURE;
}

static int Get_grid_nodes_per_face(Bmi *self, int grid, int *nodes_per_face)
{
    return BMI_FAILURE;
}


topmodel_model * new_bmi_topmodel()  //(void)?
{
    topmodel_model *data;
    data = (topmodel_model*) malloc(sizeof(topmodel_model));
    //Init pointers to NULL
    data->Q = NULL;                    // simulated discharge
    data->Qobs = NULL;                 // observed discharge
    data->rain = NULL;                 // rainfall rate
    data->pe = NULL;                   // potential evapotranspiration
    data->contrib_area = NULL;         // contributing area
    data->stor_unsat_zone = NULL;      // storage in the unsat. zone
    data->deficit_root_zone = NULL;    // root zone storage deficit
    data->deficit_local = NULL;        // local storage deficit
    data->time_delay_histogram = NULL; // time lag of outflows due to channel routing
    data->dist_area_lnaotb = NULL;     // the distribution of area corresponding to ln(A/tanB) histo.
    data->lnaotb = NULL;               // these are the ln(a/tanB) values
    data->cum_dist_area_with_dist = NULL;  // channel cum. distr. of area with distance
    data->dist_from_outlet = NULL;     // distance from outlet to point on channel with area known
    return data;
}

Bmi* register_bmi_topmodel(Bmi *model)
{
    if (model) {
        model->data = (void*)new_bmi_topmodel();
        model->initialize = Initialize;
        model->update = Update;
        model->update_until = Update_until;
        model->finalize = Finalize;

        model->get_component_name = Get_component_name;
        model->get_input_item_count = Get_input_item_count;
        model->get_output_item_count = Get_output_item_count;
        model->get_input_var_names = Get_input_var_names;
        model->get_output_var_names = Get_output_var_names;

        model->get_var_grid = Get_var_grid;
        model->get_var_type = Get_var_type;
        model->get_var_itemsize = Get_var_itemsize;
        model->get_var_units = Get_var_units;
        model->get_var_nbytes = Get_var_nbytes;
        model->get_var_location = Get_var_location;

        model->get_current_time = Get_current_time;
        model->get_start_time = Get_start_time;
        model->get_end_time = Get_end_time;
        model->get_time_units = Get_time_units;
        model->get_time_step = Get_time_step;

        model->get_value = Get_value;
        model->get_value_ptr = Get_value_ptr;   
        model->get_value_at_indices = Get_value_at_indices;

        model->set_value = Set_value;
        model->set_value_at_indices = Set_value_at_indices;

        // New BMI extensions to support serialization
        model->get_state_var_count = Get_state_var_count;
        model->get_state_var_names = Get_state_var_names;
        model->get_state_var_types = Get_state_var_types;
        model->get_state_var_ptrs  = Get_state_var_ptrs;
        model->get_state_var_sizes = Get_state_var_sizes;
        model->set_state_var       = Set_state_var;

        model->get_grid_size = Get_grid_size;
        model->get_grid_rank = Get_grid_rank;
        model->get_grid_type = Get_grid_type;

        // N/a for grid type scalar
        model->get_grid_shape = Get_grid_shape;
        model->get_grid_spacing = Get_grid_spacing;
        model->get_grid_origin = Get_grid_origin;

        // N/a for grid type scalar
        model->get_grid_x = Get_grid_x;
        model->get_grid_y = Get_grid_y;
        model->get_grid_z = Get_grid_z;

        // N/a for grid type scalar
        model->get_grid_node_count = Get_grid_node_count;
        model->get_grid_edge_count = Get_grid_edge_count;
        model->get_grid_face_count = Get_grid_face_count;
        model->get_grid_edge_nodes = Get_grid_edge_nodes;
        model->get_grid_face_edges = Get_grid_face_edges;
        model->get_grid_face_nodes = Get_grid_face_nodes;
        model->get_grid_nodes_per_face = Get_grid_nodes_per_face;

    }

    return model;
}
