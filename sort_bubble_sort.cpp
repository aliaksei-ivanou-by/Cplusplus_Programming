void bubblesort(int *a, int n)
{
    bool sorted = true;
    while (!sorted)
    {
        sorted = true;
        for (int i = 0; i < n - 1; ++i)
        {
            if (a[i] > a[i + 1])
            {
                int tmp = a[i];
                a[i] = a[i + 1];
                sorted = false;
            }
        }
    }
}