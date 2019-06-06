# Image Threshold Processing Based on Simulated Annealing and OTSU Method
Parallel implementation of simulated annealing for Image Threshold Processing Based on OTSU ([original paper](https://www.researchgate.net/publication/300243599_Image_Threshold_Processing_Based_on_Simulated_Annealing_and_OTSU_Method)).
The PSA - Paralllel Simulated Annealing was implemented based on [Details of simulated annealing algorithm to estimate parameters of multiple current dipoles using biomagnetic data](https://www.researchgate.net/publication/3220357_Details_of_simulated_annealing_algorithm_to_estimate_parameters_of_multiple_current_dipoles_using_biomagnetic_data).

### Prerequisites
* Opencv2 - [Homepage](https://opencv.org/)
* C++11 (ou superior)

### Running the Tests
For run the application is necessary compile code or run binary file
```
<path_project>$make
```
```
<path_project>$./main_parallel <image_path>
```

Example of run test:
```
<project_dir>$./main_parallel image/pepper.png
image path: image/pepper.png
image size: 512x512

Time running: 0.0107s
[thread1] threshold : 124 -> 2121.583496
[thread2] threshold : 141 -> 2006.164307
best threshold: 124
Icon theme "breeze" not found.
writed data...
```

### Authors
* **Victor Hugo**
* **Rafael da Costa Silva** - [RafaelSilva7](https://github.com/RafaelSilva7)
* **Rogério A. M. Júnior**

### License
This project is licensed under the MIT License - see the [LICENSE.md]() file for details

### Advisor
* [Glenda Michele Botelho](http://lattes.cnpq.br/3040783410094782)