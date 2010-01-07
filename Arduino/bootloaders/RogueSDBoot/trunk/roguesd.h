

extern int16_t _fwversion;

int8_t rogueSDSync(void);
inline int16_t rogueSDVersion(void) { return _fwversion; };
int8_t rogueSDOpen(const prog_uchar *filename);
int16_t rogueSDReadln(uint8_t *tostr);
void rogueSDCloseAll(void);

