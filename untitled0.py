# -*- coding: utf-8 -*-
"""
Created on Mon Sep 23 17:29:40 2024

@author: theot
"""

import numpy as np
import matplotlib.pyplot as plt

# Définir la fonction y(x)
def y_function(x, y0):
    return y0 + np.arctan(x)

# Définir la constante y0
y0 = 10 # Vous pouvez changer cette valeur

# Générer des valeurs de x
x = np.linspace(-10, 10, 400)

# Calculer les valeurs de y
y = y_function(x, y0)

# Tracer la fonction
plt.plot(x, y, label=f'y(x) = {y0} + arctan(x)')
plt.title('Tracé de la fonction y(x) = y0 + arctan(x)')
plt.xlabel('x')
plt.ylabel('y')
plt.grid(True)
plt.legend()

# Afficher le graphique
plt.show()
