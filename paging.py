from collections import deque

#CONFIG
reference_string = [415, 305, 502, 417, 305, 415, 502, 518, 417, 305, 415, 502, 520, 518, 417, 305, 502, 415, 520, 518]
frames = 4

def fifo_page_replacement(reference_string, frame_count):
    frames = deque()  # FIFO queue
    frame_set = set() 
    faults = 0
    hits = 0
    trace = []

    for step, page in enumerate(reference_string, start=1):
        if page in frame_set:
            hits += 1
            status = "hit"
        else:
            faults += 1
            status = "fault"
            if len(frames) < frame_count:
                frames.append(page)
                frame_set.add(page)
            else:
                victim = frames.popleft()  # remove oldest page
                frame_set.remove(victim)
                frames.append(page)
                frame_set.add(page)
        trace.append((step, page, list(frames), status))

    return faults, hits, trace


def lru_page_replacement(reference_string, frame_count):

    frames = []  # list to track pages in memory front = LRU
    faults = 0
    hits = 0
    trace = []

    for step, page in enumerate(reference_string, start=1):
        if page in frames:
            hits += 1
            status = "hit"
            # move page to most recently used
            frames.remove(page)
            frames.append(page)
        else:
            faults += 1
            status = "fault"
            if len(frames) < frame_count:
                #space available
                frames.append(page)
            else:
                #evict least recently used
                frames.pop(0)
                frames.append(page)
        trace.append((step, page, list(frames), status))

    return faults, hits, trace


def optimal_page_replacement(reference_string, frame_count):
    frames = set() 
    faults = 0
    hits = 0
    trace = []

    for step, page in enumerate(reference_string, start=1):
        if page in frames:
            hits += 1
            status = "hit"
        else:
            faults += 1
            status = "fault"
            if len(frames) < frame_count:
                frames.add(page)
            else:
                # Find the page to evict
                farthest_use = -1
                victim = None
                for f in frames:
                    try:
                        next_use = reference_string[step:].index(f)  # relative to current step
                    except ValueError:
                        # page not used again evict immediately
                        victim = f
                        break
                    if next_use > farthest_use:
                        farthest_use = next_use
                        victim = f
                frames.remove(victim)
                frames.add(page)
        trace.append((step, page, list(frames), status))

    return faults, hits, trace

def main():
    #FIFO Algorithm
    print("Testing FIFO")

    faults, hits, trace = fifo_page_replacement(reference_string, frames)

    print(f"FIFO -> Page Faults: {faults}, Hits: {hits}, Hit Ratio: {hits/len(reference_string):.3f}\n")

    print("Step | Page | Frames after access | Status")
    for step, page, fr, status in trace:
        print(f"{step:4} | {page:3} | {fr} | {status}")

    #LRU Algorithm
    print("Testing LRU")
    faults, hits, trace = lru_page_replacement(reference_string, frames)

    print(f"LRU -> Page Faults: {faults}, Hits: {hits}, Hit Ratio: {hits/len(reference_string):.3f}\n")

    print("Step | Page | Frames after access | Status")
    for step, page, fr, status in trace:
        print(f"{step:4} | {page:3} | {fr} | {status}")


    #Optimal Algorithm
    print("Testing Optimal")
    faults, hits, trace = optimal_page_replacement(reference_string, frames)

    print(f"OPT -> Page Faults: {faults}, Hits: {hits}, Hit Ratio: {hits/len(reference_string):.3f}\n")

    print("Step | Page | Frames after access | Status")
    for step, page, fr, status in trace:
        print(f"{step:4} | {page:3} | {fr} | {status}")
    
    return 0

status = main()
exit(status)
