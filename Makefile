.PHONY: test tiles
# Host-side tests of the shared game core (no console toolchain needed)
test:
	gcc -O1 -g -fsanitize=address,undefined -DPLAT_GB  -Icommon common/game.c test/sim.c -o /tmp/sim_gb  && /tmp/sim_gb
	gcc -O1 -g -fsanitize=address,undefined -DPLAT_SMS -Icommon common/game.c test/sim.c -o /tmp/sim_sms && /tmp/sim_sms
tiles:
	python3 tools/gen_tiles.py
