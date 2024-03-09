This algorithm is centered around predicting the potential ratings that specific individuals may assign to particular films. It employs the concept of similarity to achieve this. The process involves taking the average rating given to a film by the people who are most similar to the individual for whom the prediction is being made, based on the training data. These similar individuals are identified through their highest similarity scores. The operation of the algorithm is as follows:

1. Data is stored in a large matrix using a pointer mechanism, structured akin to a hashmap or dictionary for efficient data retrieval. This setup allows for placements based on people's IDs, making the time complexity O(n). For example, the rating given by a person with ID 5 to a movie with ID 15 is located at matrix[5][15].

2. The person and film for which a rating prediction is needed are identified using the submission file data.

3. A cosine similarity calculation is performed to find the k number of most similar individuals to the person in question. This similarity is determined based on matching movies that the individual has watched.

4. The average rating for the movie, as rated by these k similar individuals, is calculated for the prediction.

5. This predicted rating is then recorded in the rating submission file.

6. The process repeats for the next individual and movie requiring a rating prediction.
