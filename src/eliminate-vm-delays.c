#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>

double get_time_in_seconds() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec + time.tv_usec * 1e-6;
}

int main(int argc, char **argv) {
    bool verbose = false;
    int fps = 60;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "--fps") == 0) {
            if (i + 1 < argc) {
                fps = atoi(argv[++i]);
                if (fps <= 0) {
                    fprintf(stderr, "Invalid FPS value. Using default 60 FPS.\n");
                    fps = 60;
                }
            } else {
                fprintf(stderr, "No FPS value specified after --fps. Using default 60 FPS.\n");
                fps = 60;
            }
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s [--verbose] [--fps <frames per second>]\n", argv[0]);
            return -1;
        }
    }

    // Calculate sleep time in microseconds
    int sleep_time_us = (int)(1e6 / fps);

    EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDpy == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display\n");
        return -1;
    }

    if (!eglInitialize(eglDpy, NULL, NULL)) {
        fprintf(stderr, "Failed to initialize EGL\n");
        return -1;
    }

    static const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLConfig eglCfg;
    EGLint numConfigs;
    if (!eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs) || numConfigs != 1) {
        fprintf(stderr, "Failed to choose EGL config\n");
        eglTerminate(eglDpy);
        return -1;
    }

    static const EGLint pbufferAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE,
    };

    // Create an off-screen surface (pbuffer)
    EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, pbufferAttribs);
    if (eglSurf == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to create EGL surface\n");
        eglTerminate(eglDpy);
        return -1;
    }

    static const EGLint ctxAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, ctxAttribs);
    if (eglCtx == EGL_NO_CONTEXT) {
        fprintf(stderr, "Failed to create EGL context\n");
        eglDestroySurface(eglDpy, eglSurf);
        eglTerminate(eglDpy);
        return -1;
    }

    if (!eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx)) {
        fprintf(stderr, "Failed to make EGL context current\n");
        eglDestroyContext(eglDpy, eglCtx);
        eglDestroySurface(eglDpy, eglSurf);
        eglTerminate(eglDpy);
        return -1;
    }

    double start_time = get_time_in_seconds();
    int frame_count = 0;

    while (1) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Simple rendering operation
        glViewport(0, 0, 1, 1);

        // Define vertex data for a simple triangle
        GLfloat vertices[] = {
            0.0f,  1.0f, // Vertex 1 (X, Y)
           -1.0f, -1.0f, // Vertex 2 (X, Y)
            1.0f, -1.0f  // Vertex 3 (X, Y)
        };

        // Create a vertex buffer
        GLuint VBO;
        glGenBuffers(1, &VBO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // Upload vertex data to the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);

        // Specify how the data for position can be accessed
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Draw the triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDisableVertexAttribArray(0);

        glDeleteBuffers(1, &VBO);

        glFlush();

        frame_count++;

        // Calculate and print FPS every second if verbose
        if (verbose) {
            double current_time = get_time_in_seconds();
            if (current_time - start_time >= 1.0) {
                printf("FPS: %d\n", frame_count);
                fflush(stdout);
                frame_count = 0;
                start_time = current_time;
            }
        }

        // Sleep to control frame rate
        usleep(sleep_time_us);
    }

    // Cleanup (unreachable)
    eglDestroyContext(eglDpy, eglCtx);
    eglDestroySurface(eglDpy, eglSurf);
    eglTerminate(eglDpy);

    return 0;
}
