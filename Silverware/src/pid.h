
float pid( int x );
int next_pid_term( void); // Return value : 0 - p, 1 - i, 2 - d
int next_pid_axis( void); // Return value : 0 - Roll, 1 - Pitch, 2 - Yaw
int increase_pid( void );
int decrease_pid( void );
void pid_precalc( void);
float get_hard_coded_pid_identifier( void);
void read_pids_from_mem( void);
void write_pids_to_mem( void);
void fmc_write_pids(float * pids_array[]);
int hardcoded_pids_changed( void);
