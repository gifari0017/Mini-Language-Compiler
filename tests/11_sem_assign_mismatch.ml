// SEMANTIC ERROR (d, bonus): assignment type mismatches
int a = "hello";        // cannot assign string to int
bool b = 5;             // cannot assign int to bool
string s = 3.14;        // cannot assign float to string
int c = 1;
c = true;               // cannot assign bool to int
float ok = 7;           // ALLOWED: int widens to float (no error)
