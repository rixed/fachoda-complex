static inline int mapmapm(int k) {
	if (mapmap[k]&0x80) return 0;
	else return mapmap[k];
}
