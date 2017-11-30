#!/bin/bash

for i in ./hw1_sample_scenes/*.xml; do ./raytracer "$i"; done

