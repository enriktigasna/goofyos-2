global sched_idle

sched_idle:
	sti
loop:
	hlt
	jmp loop