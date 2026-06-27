.PHONY: clean launch

CONFIG ?= config/config_full_mock.toml

clean:
	rm -rf smart_aquaria/build

launch:
	python3 -m aquarunner $(CONFIG) $(BUILD_TYPE)
