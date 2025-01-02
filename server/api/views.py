import re
from datetime import datetime
from rest_framework import status
from rest_framework.decorators import api_view
from rest_framework.response import Response
from django.core.exceptions import ValidationError
from .models import SensorData
from .serializer import SensorDataSerializer

# Create your views here.
@api_view(['GET'])
def get_data(request):
    from_timestamp = request.query_params.get('from', None)
    
    if from_timestamp:    
        try:
            from_time = datetime.fromisoformat(from_timestamp)
            sensor_data = SensorData.objects.filter(timestamp__gte=from_time)
        except ValueError:
            return Response("[ERROR] Invalid timestamp format. Use ISO 8601 format.", status=status.HTTP_400_BAD_REQUEST)
    else:
        sensor_data = SensorData.objects.all()
    
    try:        
        serialized_data = SensorDataSerializer(sensor_data, many=True).data
        return Response(serialized_data, status=status.HTTP_200_OK)
    except SensorData.DoesNotExist:
        return Response("[ERROR] Data not found", status=status.HTTP_404_NOT_FOUND)
    except ValidationError as e:
        return Response(f"[ERROR] validation error: {str(e)}", status=status.HTTP_400_BAD_REQUEST)
    except Exception as e:
        return Response(f"[ERROR] an unexpected error occured: {str(e)}", status=status.HTTP_500_INTERNAL_SERVER_ERROR)
    
@api_view(['POST'])
def post_data(request):
    try:
        body = request.body.decode('utf-8')

        pattern = r"([\w_]+):([-+]?\d*\.?\d+)"
        pairs = re.findall(pattern, body)

        data = {key: float(value) if '.' in value else int(value) for key, value in pairs}
        if "ugm3_25um" in data:
            data["ugm3_25um"] *= 1000
        if "ugm3_1um" in data:
            data["ugm3_1um"] *= 1000

        # print(data)
        serializer = SensorDataSerializer(data=data)
        if serializer.is_valid():
            serializer.save()
            return Response("[SUCCESS]", status=status.HTTP_201_CREATED)
        return Response(f"[ERROR] Validation Failed: {serializer.errors}", status=status.HTTP_400_BAD_REQUEST)
    except ValidationError as e:
        return Response(f"[ERROR] Validation error: {str(e)}", status=status.HTTP_400_BAD_REQUEST)

    except UnicodeDecodeError as e:
        return Response(f"[ERROR] Unable to decode request body: {str(e)}", status=status.HTTP_400_BAD_REQUEST)

    except KeyError as e:
        return Response(f"[ERROR] Missing required key: {str(e)}", status=status.HTTP_400_BAD_REQUEST)

    except TypeError as e:
        return Response(f"[ERROR] Type error in data: {str(e)}", status=status.HTTP_400_BAD_REQUEST)

    except Exception as e:
        return Response(f"[ERROR] An unexpected error occurred: {str(e)}", status=status.HTTP_500_INTERNAL_SERVER_ERROR)