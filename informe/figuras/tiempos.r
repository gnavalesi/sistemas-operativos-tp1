library(dplyr)
library(ggplot2)
library(tidyr)
library(readr)
library(scales)

theme_set(theme_minimal())

tiempos <- read_csv("tiempos.csv")

tiempos %>% 
  mutate(maximum_rel_time = maximum_time / maximum2_time) %>% 
  ggplot(aes(x = p_archivos, y = maximum_rel_time)) +
  geom_col(aes(fill = as.factor(p_maximos)), position = "dodge") +
  scale_fill_brewer(palette = "Paired") +
  theme(legend.position = "bottom") +
  coord_cartesian(ylim = c(0.75, 1.75)) +
  labs(x = "p_archivos", y = "Tiempo relativo", fill = "p_maximos")

ggsave("tiempos.pdf", device = "pdf", units = "cm", height = 8, width = 15)
