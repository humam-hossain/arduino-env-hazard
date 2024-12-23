import re
from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
from .models import SensorData

# Create your views here.
def get_data(request):
    if request.method == 'GET':
        try:    
            latest_data = SensorData.objects.last()
            data = (
                f"timestamp:{latest_data.timestamp} "
                f"t:{latest_data.t} "
                f"samples:{latest_data.samples} "
                f"r[25um]:{latest_data.r_25um} "
                f"ug/m3[25um]:{latest_data.ugm3_25um} "
                f"pcs[25um]:{latest_data.pcs_25um} "
                f"r[1um]:{latest_data.r_1um} "
                f"ug/m3[1um]:{latest_data.ugm3_1um} "
                f"pcs[1um]:{latest_data.pcs_1um}"
            )
            return HttpResponse(data, content_type="text/plain", status=200)
        except SensorData.DoesNotExist:
            return HttpResponse("ERROR", content_type="text/plain", status=404)
    return HttpResponse("INVALID", content_type="text/plain", status=400)

@csrf_exempt
def post_data(request):
    if request.method == 'POST':
        try:
            body = request.body.decode('utf-8')
            
            pattern = r"([\w_]+):([-+]?\d*\.?\d+)"
            pairs = re.findall(pattern, body)
            
            data = {key: float(value) if '.' in value else int(value) for key, value in pairs}
            data["ugm3_25um"] *= 1000
            data["ugm3_1um"] *= 1000
            
            # print(data)
            SensorData.objects.create(**data)
            
            return HttpResponse("SUCCESS", content_type="text/plain", status=200)
        except Exception:
            return HttpResponse("ERROR", content_type="text/plain", status=400)