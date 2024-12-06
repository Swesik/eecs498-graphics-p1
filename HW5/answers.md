
# GAN
## Task 2:
### Question:
Please explain what is the difference between $E_{z∼p(Z)}$ term in Eq. 2 and Eq. 1? And why there is such a difference between the Generator and the Discriminator? Briefly explain the difference between the objective of the Generator and the Discriminator loss function is adequate.

### Answer:
In the case of $\mathcal{L}_\mathcal{G}$, the $E_{z∼p(z)}$ term quantifies how poor the Generator is at fooling the discriminator. On the other hand, the $E_{z∼p(z)}$ term in $\mathcal{L}_\mathcal{D}$ quantifies how poor the discriminator is at distinguishing the generator's output as a fake image.

## Task 3 (optional):
### Question:
Despite the two regularization techniques we have shown above, what are some other
augmentation or regularization techniques that are functional in GAN? Please search for the literature online and list
three or more here.

### Answer:
WGAN-GP Loss
Spectral Normalization
Label Smoothing
Consistency Regularization

## Task 5 (optional):
### Question: 
Checkout lines 56-59 of training/loss.py. Think about why we only need to modify
one part and it is enough to handle three cases shown in Fig. 5. Share your thoughts here.

### Answer:
Let us consider the three cases. By running DiffAug on all inputs passed into the discriminator we handle both the case where real data and fake data is passed into the discriminator. In other words, the discriminator will be of the form $D(T(x))$ for all inputs. For the last case of applying Diff aug to the generator output, since the output passes through the discriminator to get the generator loss, the discriminator will apply diff aug on the output of G so we do not need to apply DiffAug again to the output of the generator.


# Diffusion
## Task3:
### Question:
Please report the time you need for DDPM and DDIM respectively and briefly talk about the pros and cons of
using DDIM as the scheduler over DDPM. One sentence response is adequate.

DDPM took 341.2 seconds while DDIM took 17.6 seconds. DDPM is better at creating a more diverse set of images, but DDIM tends to create cleaner images with less noise on average. DDIM is also able to converge with a smaller number of samples and is deterministic. 

## Task5:
### Question:
What is the extra time cost for applying the DPS compared to DDIM? What will happen if we set
fewer timesteps than usual (look at TODO in execute_DPS.py)? Please run the experiment in different timesteps and share your findings. The response can be as short as one sentence. Sticking some figures is recommended.

### Answer
We see that DPS seems to scale linearly with the number of samples. The extra time cost comes from calculating the norm of $y - A(x_0)$ and the gradient of this function w.r.t. $x_t$.

Here is a list of iterations for DPS compared against the time to run this many iterations:
- 50 iterations --> 24.71972393989563 seconds
- 300 iterations --> 63.30282139778137 seconds 
- 600 iterations --> 126.09506916999817 seconds
- 100 iterations --> 21.404398918151855 seconds
- 800 iterations --> 168.45827221870422 seconds
- 1000 iterations --> 209.9075665473938 seconds
