def request_handler(request):
    if request["method"] == "POST":
        correlations = {}
        correlations["left"] = float(request["form"]["left"])
        correlations["right"] = float(request["form"]["right"])
        correlations["up"] = float(request["form"]["up"])
        correlations["down"] = float(request["form"]["down"])
        direction =  max(correlations, key=lambda key: correlations[key])
        if correlations[direction] > 0.3:
            return direction
        else:
            return "-1"