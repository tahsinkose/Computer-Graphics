#!/bin/bash

for i in ./hw2_sample_scenes/*.xml; do ./raytracer "$i"; done

