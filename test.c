#include <stdio.h>
#include <stdlib.h>
#include "src/Vokaturi.h"
#include <pthread.h>


int main(int argc, char *argv[])
{

	printf("hello world\n");
	return 0;

}

#define SHARED_BUFFER_SIZE  220500
struct {
    double samples [SHARED_BUFFER_SIZE];   // so that's approximately 1.76 megabytes
    int64_t numberOfReceivedSamples;
    int64_t numberOfSentSamples;
} sharedBuffer;

VokaturiVoice ourVoice;   // initialize at start-up
#define Lock pthread_mutex_t
struct Lock ourLock;


#define Lock(x) pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER
#define Lock(x) pthread_mutex_lock(x)
#define unlock(x) pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER
#define Lock(x) pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER
#define Lock(x) pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER
Lock ourLock;

void recordingCallback (int numberOfSamples, int16_t samples) {
    lock (ourLock);
    int64_t sampleCount = sharedBuffer.numberOfReceivedSamples;
    int32_t samplePointer = sampleCount % SHARED_BUFFER_SIZE;
    for (int32_t i = 0; i < numberOfSamples; i ++) {
        if (samplePointer >= SHARED_BUFFER_SIZE)
            samplePointer -= SHARED_BUFFER_SIZE;
        sharedBuffer.samples [samplePointer] = (double) samples [i];
        samplePointer += 1;
    }
    unlock (ourLock);
}
void timerCallback () {
    lock (ourLock);
    if (sharedBuffer.numberOfReceivedSamples == 0) {
        unlock (ourLock);
        return;   // nothing recorded yet
    }
    if (sharedBuffer.numberOfReceivedSamples > sharedBuffer.numberOfSentSamples) {
        for (int64_t isamp = sharedBuffer.numberOfSentSamples;
             isamp < sharedBuffer.numberOfReceivedSamples;
             isamp ++)
        {
            int32_t indexOfSampleToBeSent = (int32_t) (isamp % SHARED_BUFFER_SIZE);
            VokaturiVoice_fill (ourVoice, 1,
                                & sharedBuffer.samples [indexOfSampleToBeSent]);
        }
        sharedBuffer.numberOfSentSamples = sharedBuffer.numberOfReceivedSamples;
    }
    unlock (ourLock);
    VokaturiQuality quality;
    VokaturiEmotionProbabilities emotionProbabilities;
    VokaturiVoice_extract (ourVoice, & quality, & emotionProbabilities);
    if (quality.valid) {
        printf ("%.6f %.6f %.6f %.6f %.6f\n",
            emotionProbabilities.neutrality,
            emotionProbabilities.happiness,
            emotionProbabilities.sadness,
            emotionProbabilities.anger,
            emotionProbabilities.fear);
    }
}
